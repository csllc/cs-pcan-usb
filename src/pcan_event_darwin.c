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
#include <pthread.h>     // provide pthread_create, pthread_join, and types
#include <unistd.h>      // provide pipe
#include <string.h>      // provide memcpy
#include <stdlib.h>      // provide malloc and free
#include <PCBUSB.h>      // provide PCAN-Basic types, constants, and functions
#endif

#include "pcan.h"        // provide default callback function
#include "pcan_event_darwin.h"
#include "pcan_helper.h" // provide pcanStatusLookup


// Pipes are used here instead of condition variables because the PCBUSB library
// provides event notifications via the read end of a pipe, which is expected
// to be waited on using select(2). Both pcanPipeExit[R] and pcanPipeRead
// (pipe read ends) can be put into an fd_set and waited on by the same
// select(2) call. This is similar to the use of WaitForMultipleObjects in
// the Win32 version.


// ----------------------------------- // -----------------------------------
// Definitions

// Constants for accessing arrays of pipe file descriptors below. Following
// convention, the read end of the pipe is first ([0]) and the write end is
// second ([1]).
enum { R, W };




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables

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
    pcanDumpThreadParameters(&threadParams);
#endif

    // Signal the spawn pipe, indicating that the worker thread is running
    size_t count = 0;
    int buf = 0;
    count = write(threadParams.pipeSpawn, &buf, sizeof(buf));
    if (count != sizeof(buf))
    {
        printf("pcanEventThreadProc: Error at write(threadParams.pipeSpawn)\n");
        return (void*)1;
    }

    // Set up file descriptors for select
    fd_set readfds;
    FD_ZERO(&readfds);
	FD_SET(threadParams.pipeRead, &readfds);
    FD_SET(threadParams.pipeExit, &readfds);

    // Determine largest file descriptor
    int nfds = 0;
    nfds = (threadParams.pipeRead > threadParams.pipeExit) ?
        threadParams.pipeRead : threadParams.pipeExit;

    // Wait for either file descriptor to be signaled
    int threadExit = 0;

    while (threadExit == 0)
    {
#ifdef PCAN_EVENT_DARWIN_DEBUG
        printf("pcanEventThreadProc: Waiting for pipeRead or pipeExit...\n");
#endif
        ret = select(nfds+1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(threadParams.pipeRead, &readfds))
        {
            // Invoke callback
            (threadParams.callback)(threadParams.canChannel);
        }
        else if (FD_ISSET(threadParams.pipeExit, &readfds))
        {
            // Clean up and exit thread
#ifdef PCAN_EVENT_DARWIN_DEBUG
            printf("pcanEventThreadProc: Exiting thread.\n");
#endif
            threadExit = 1;
        }
        else
        {
            printf("pcanEventThreadProc: Unexpected value returned by "
                   "select(): %i. Exiting thread.\n", ret);
            return (void*)1;
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
    if (pcanEventThreadParams == 0)
    {
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
    
    printf("pcanEventEnable: Waiting for thread to start...\n");
    ret = select(pcanPipeSpawn[R]+1, &readfds, NULL, NULL, NULL);

    // Cleanup
    free(pcanEventThreadParams);

    return 0;
}




int pcanEventDisable(TPCANHandle pcan_Channel)
{
    int ret = 0;

    // Stop the worker thread
    size_t count = 0;
    int buf = 0;
    count = write(pcanPipeExit[W], &buf, sizeof(buf));
    if (count != sizeof(buf))
    {
        printf("pcanEventDisable: Error at write(pcanPipeExit)\n");
        return 1;
    }

    // Wait for the worker thread to exit
    void *pcanEventThreadExitCode = 0;

    ret = pthread_join(pcanEventThread, &pcanEventThreadExitCode);
    if (ret != 0)
    {
        printf("pcanEventDisable: Error at pthread_join\n");
    }
    else
    {
        printf("pcanEventDisable: Worker thread exited with code %i\n",
               (int)pcanEventThreadExitCode);
    }

    // Close pipes
    close(pcanPipeSpawn[W]);
    close(pcanPipeSpawn[R]);
    close(pcanPipeExit[R]);
    close(pcanPipeExit[W]);

    return 0;
}




void pcanDumpThreadParameters(threadParameters_t *threadParameters)
{
    printf("threadParameters {\n"
           "  canChannel  = 0x%02X\n"
           "  callback    = %p\n"
           "  pipeRead    = %i\n"
           "  pipeSpawn   = %i\n"
           "  pipeExit    = %i\n"
           "}\n",
           threadParameters->canChannel,
           threadParameters->callback,
           threadParameters->pipeRead,
           threadParameters->pipeSpawn,
           threadParameters->pipeExit);

    return;
}

