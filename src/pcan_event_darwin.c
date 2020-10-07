/* PCAN-Basic event control functions and worker thread process

   Functions that enable and disable the use of an event used by the PCAN-Basic
   library to signal that received data is available for reading. When enabled,
   a worker thread is spawned that waits on this event and invokes a callback
   function when its state changes to signaled, which presumably calls 
   CAN_Read() to retrieve the newly received data.

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/

#include <stdio.h>       // provide printf
#include <stdbool.h>     // provide boolean values

#if defined _WIN32
#include <windows.h>     // provide Win32 types, constants, and functions
#include <PCANBasic.h>   // provide PCAN-Basic types, constants, and functions
#elif defined __APPLE__
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h> // provide memcpy
#include <stdlib.h> // provide malloc and free
#include <PCBUSB.h>      // provide PCAN-Basic types, constants, and functions
#endif

#include "pcan.h"        // provide default callback function
#include "pcan_event_darwin.h"
#include "pcan_helper.h" // provide pcanStatusLookup

#include <fcntl.h> // provide fcntl
#include <sys/syslimits.h> // provide PATH_MAX
#include <errno.h>

#include <unistd.h> // provide pipe


// ----------------------------------- // -----------------------------------
// Definitions




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables

// Constants for accessing arrays of pipe file descriptors below. Following
// convention, the read end of the pipe is first ([0]) and the write end is
// second ([1])
const enum PIPE { R, W }; // TODO hopefully no namespace collision here

// Pipe fd for CAN bus read event, signaled my main thread (PCAN-Basic API) when
// a message has been received and waited on by worker thread
int pcanPipeRead = 0;

// Pipe file descriptors for worker thread spawn signaling,
// signaled by worker shortly after thread starts and waited on by main thread.
int pcanPipeSpawn[2] = { 0 };

// Pipe file descriptors for worker thread exit signaling,
// signaled by main thread during exit routine, and waited on by worker thread
int pcanPipeExit[2] = { 0 };

// Worker thread ID, populated by pcanEventEnable
pthread_t pcanEventThread = 0;




// ----------------------------------- // -----------------------------------
// Local functions


void *pcanEventThreadProc(void *lpParam)
{
    int ret = 0;

    // Copy parameters passed from the main thread
    threadParameters_t threadParams = { 0 };
    memcpy(&threadParams, (threadParameters_t*)lpParam, sizeof(threadParams));

#ifdef PCAN_EVENT_DARWIN_DEBUG
    pcanDumpThreadParametersDarwin(&threadParams);
#endif

    // Signal the spawn pipe, indicating that the worker thread is running
    size_t count = 0;
    int buf = 0;
    count = write(threadParams.pipeSpawn, &buf, sizeof(buf));
    if (count != sizeof(buf))
    {
        printf("pcanEventThreadProc: Error at write(threadParams.pipeSpawn)\n");
        return 1;
    }

    // Set up file descriptors for select
    fd_set readfds;
    FD_ZERO(&readfds);
	FD_SET(threadParams.pipeRead, &readfds);
    FD_SET(threadParams.pipeExit, &readfds);

    TPCANMsg message = { 0 };
    TPCANStatus pcanStatus;

    // Wait for either file descriptor to be signaled
    int threadExit = 0;

    while (threadExit == 0)
    {
#ifdef PCAN_EVENT_DARWIN_DEBUG
        printf("pcanEventThreadProc: Waiting for pipeRead or pipeExit...\n");
#endif
        ret = select(threadParams.pipeExit+1, &readfds, NULL, NULL, NULL);

        // TODO Read from the pipe and flush it out? Is it a problem?

        if (FD_ISSET(threadParams.pipeRead, &readfds))
        {
            // Invoke callback
            (threadParams.callback)(threadParams.canChannel);
        }
        else if (FD_ISSET(threadParams.pipeExit, &readfds))
        {
            // Clean up and exit thread
#ifdef PCAN_EVENT_DEBUG
            printf("pcanEventThreadProc: Exiting thread\n");
#endif
            threadExit = 1;
        }
        else
        {
            printf("pcanEventThreadProc: Unexpected value returned by "
                   "select(): %i. Exiting thread.\n", ret);
            return 1;
        }
    }

    printf("pcanThreadProc: Exiting.\n");
    return 0;
}




// ----------------------------------- // -----------------------------------
// Public functions


int pcanEventEnable(TPCANHandle pcanChannel, void *callback)
{
    int ret = 0;

    TPCANStatus pcanResult = 0;

    // Retreive the pipe file descriptor that the PCANBasic API will use to
    // communciate CAN read events with the worker thread
    pcanResult = CAN_GetValue(pcanChannel, PCAN_RECEIVE_EVENT,
                              &pcanPipeRead, sizeof(int));

    if (pcanResult != PCAN_ERROR_OK)
    {
        printf("pcanEventEnable: Error at CAN_GetValue: 0x%02X (%s)\n",
               pcanResult, pcanStatusLookup(pcanResult));
    }

    // TODO: Be sure to uninitialize PCAN device if it's already initialized
    // at startup

    // We're using pipes here instead of condition variables because
    // they can all be passed to the same select() call in the worker thread

    // Create pipe that worker thread will use to signal it has started
    ret = pipe(pcanPipeSpawn);
    if (ret == -1)
    {
        printf("pcanEventEnable: Error at pipe(pcanPipeSpawn): 0x%02X\n",
               ret);
        return 1;
    }

    // Create pipe that main thread will use to stop worker thread
    ret = pipe(pcanPipeExit);
    if (ret == -1)
    {
        printf("pcanEventEnable: Error at pipe(pcanPipeExit): 0x%02X\n",
               ret);
        return 1;
    }

    // Pack parameters into struct to be passed to worker thread
    threadParameters_t *pcanEventThreadParams = 0;
    pcanEventThreadParams = malloc(sizeof(*pcanEventThreadParams));
    if (pcanEventThreadParams == 0) {
        printf("pcanEventEnable: Error at malloc.\n");
        return 1;
    }

    pcanEventThreadParams->canChannel = pcanChannel;
    pcanEventThreadParams->callback = callback;
    pcanEventThreadParams->pipeRead = pcanPipeRead;
    pcanEventThreadParams->pipeSpawn = pcanPipeSpawn[W];
    pcanEventThreadParams->pipeExit = pcanPipeExit[R];

    // Spawn the worker thread that will monitor the event
    ret = pthread_create(&pcanEventThread, // thread
                         NULL, // attr
                         &pcanEventThreadProc, // start_routine
                         pcanEventThreadParams); // arg
    if (ret != 0)
    {
        printf("pcanEventEnable: Error at pthread_create: 0x%02X\n",
               ret);
        return 1;
    }

    // Wait for pcanEventSpawn to be signaled and all that...
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(pcanPipeSpawn[R], &readfds);
    
    printf("pcanEventEnable: Waiting for thread to spawn...\n");
    ret = select(pcanPipeSpawn[R]+1, &readfds, NULL, NULL, NULL);
    printf("pcanEventEnable: Select exited with %i\n", ret);

    // Cleanup
    free(pcanEventThreadParams);

    return 0;
}




int pcanEventDisable(TPCANHandle pcan_Channel)
{

    return 0;
}
