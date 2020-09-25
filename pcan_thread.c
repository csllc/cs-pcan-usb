#include <stdbool.h>
#include <stdio.h>
#include <windows.h>

#include "pcan_thread.h"
#include "pcan_helper.h"

DWORD WINAPI pcan_thread_proc(LPVOID lpParam)
{
    int ret = 0;

    /* Copy parameters passed from the main thread */
    threadParameters_t thread_params = { 0 };
    memcpy(&thread_params, (threadParameters_t*)lpParam, sizeof(thread_params));

    /* Open handles for events */
    HANDLE pcan_read_event = INVALID_HANDLE_VALUE;
    HANDLE pcan_spawn_event = INVALID_HANDLE_VALUE;
    HANDLE pcan_exit_event = INVALID_HANDLE_VALUE;
    
    pcan_read_event = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, /* dwDesiredAccess */
				false, /* bInheritHandle */
				"pcan_read_event"); /* TODO: Make these constants somewhere */

    if (pcan_read_event == 0) {
	printf("pcan_thread_proc: Error at OpenEvent(pcan_read_event): %i\n",
	       GetLastError());
	return 1;
    }

    pcan_spawn_event = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, /* dwDesiredAccess */
				 false, /* bInheritHandle */
				 "pcan_spawn_event");

    if (pcan_spawn_event == 0) {
	printf("pcan_thread_proc: Error at OpenEvent(pcan_spawn_event): %i\n",
	       GetLastError());
	return 1;
    }

    pcan_exit_event = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, /* dwDesiredAccess */
				false, /* bInheritHandle */
				"pcan_exit_event");

    if (pcan_exit_event == 0) {
	printf("pcan_thread_proc: Error at OpenEvent(pcan_exit_event): %i\n",
	       GetLastError());
	return 1;
    }

    /* Set spawn_event, indicating that the worker thread is running */
    ret = SetEvent(pcan_spawn_event);
    if (ret == 0) {
	printf("pcan_thread_proc: Error at SetEvent (0x%02X)\n", GetLastError());
	return 1;
    }
    
    /* Wait for either event to be signaled */
    int thread_exit = 0;
    HANDLE events[2] = { 0 };
    int event_index = 0;
    
    events[0] = pcan_read_event; /* TODO: Use constants for indices */
    events[1] = pcan_exit_event;

    while (thread_exit == 0) {
	//printf("pcan_thread_proc: Waiting for read_event or thread_event...\n");
	ret = WaitForMultipleObjects(2, /* nCount */
				     events, /* lpHandles */
				     false, /* bWaitAll */
				     INFINITE); /* dwMilliseconds */

	if (ret & WAIT_ABANDONED_0) {
	    event_index = (ret ^ WAIT_ABANDONED_0);
	    printf("pcan_thread_proc: WAIT_ABANDONED_0 %i\n", event_index);
	} else if (ret == WAIT_TIMEOUT) {
	    printf("pcan_thread_proc: WAIT_TIMEOUT\n");
	    return 1;
	} else if (ret == WAIT_FAILED) {
	    printf("pcan_thread_proc: WAIT_FAILED: %i\n", GetLastError());
	    return 1;
	} else { /* ret == (WAIT_OBJECT_0 | event_index) and WAIT_OBJECT_0 = 0 */
	    switch (ret) {
	    case 0: /* TODO: Use constants here */
		(thread_params.callback)(thread_params.canChannel);
		break;
	    case 1:
		printf("pcan_thread_proc: Exiting thread\n");
		CloseHandle(pcan_read_event);
		CloseHandle(pcan_spawn_event);
		CloseHandle(pcan_exit_event);
		thread_exit = 1;
	    }
	}
    }

    return 0;
}


