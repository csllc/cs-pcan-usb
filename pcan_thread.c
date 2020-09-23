#include <stdbool.h>
#include <stdio.h>
#include <windows.h>

#include "pcan_thread.h"
#include "pcan_helper.h"

DWORD WINAPI pcan_thread_proc(LPVOID lpParam)
{
    int ret = 0;

    /* Cast the parameter passed from the main thread */
    int can_channel = (int)lpParam;

    HANDLE pcan_read_event = INVALID_HANDLE_VALUE;
    HANDLE pcan_thread_event = INVALID_HANDLE_VALUE;
    
    pcan_read_event = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, /* dwDesiredAccess */
				false, /* bInheritHandle */
				"pcan_read_event");

    if (pcan_read_event == 0) {
	printf("pcan_thread_proc: Error at OpenEvent(pcan_read_event): %i\n",
	       GetLastError());
	return 1;
    }

    pcan_thread_event = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, /* dwDesiredAccess */
				  false, /* bInheritHandle */
				  "pcan_thread_event");

    if (pcan_thread_event == 0) {
	printf("pcan_thread_proc: Error at OpenEvent(pcan_thread_event): %i\n",
	       GetLastError());
	return 1;
    }

    /* Signal the thread event, indicating that the worker thread is running */
    ret = SetEvent(pcan_thread_event);
    if (ret == 0) {
	printf("pcan_thread_proc: Error at SetEvent (0x%02X)\n", GetLastError());
	return 1;
    }
    
    /* Wait for either event to be signaled */
    int thread_exit = 0;
    HANDLE events[2] = { 0 };
    int event_index = 0;
    
    events[0] = pcan_read_event;
    events[1] = pcan_thread_event;

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
	} else {
	    switch (ret) {
	    case 0: /* TODO: Use constants here */
		pcan_thread_read(can_channel);
		break;
	    case 1:
		printf("pcan_thread_proc: Exiting thread\n");
		CloseHandle(pcan_read_event);
		CloseHandle(pcan_thread_event);
		return 0;
	    }
	}
    }

    return 0;
}




void pcan_thread_read(int channel)
{
    TPCANStatus status = PCAN_ERROR_OK;
    TPCANMsg msg = { 0 };
    TPCANTimestamp timestamp = { 0 } ;
    
    do {
	ZeroMemory(&msg, sizeof(msg));
	ZeroMemory(&timestamp, sizeof(timestamp));

	status = CAN_Read(channel, &msg, &timestamp);

	if (status == PCAN_ERROR_OK) {
	    //printf("---\n");
	    pcan_dump_msg(&msg);
	    //pcan_dump_timestamp(&timestamp);
	    //printf("status: %i (%s)\n", status, pcan_status_lookup(status));
	}
    } while (status == PCAN_ERROR_OK);
    
    return;
}

