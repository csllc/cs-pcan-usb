#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "PCANBasic.h"
#include "pcan_event.h"
#include "pcan_thread.h"
#include "pcan_helper.h"

HANDLE pcan_read_event = INVALID_HANDLE_VALUE;
HANDLE pcan_thread_event = INVALID_HANDLE_VALUE;
HANDLE pcan_thread = INVALID_HANDLE_VALUE;
DWORD pcan_thread_id = 0;

int pcan_EnableEvent(TPCANHandle pcan_Channel)
{
    int ret = 0;

    /* Create security attributes structure for events and worker thread */
    SECURITY_DESCRIPTOR pcan_security_desc = { 0 };

    ret = InitializeSecurityDescriptor(&pcan_security_desc,
				       SECURITY_DESCRIPTOR_REVISION);
    if (ret == 0) {
	printf("pcan_EnableEvent: Error at InitializeSecurityDescriptor: %i\n",
	       GetLastError());
	return 1;
    }
    
    ret = SetSecurityDescriptorDacl(&pcan_security_desc,
				    true,
				    0,
				    false);
    if (ret == 0) {
	printf("pcan_EnableEvent: Error at SetSecurityDescriptorDacl: %i\n",
	       GetLastError());
	return 1;
    }

    SECURITY_ATTRIBUTES pcan_security_attr = { 0 };
    pcan_security_attr.nLength = sizeof(pcan_security_attr);
    pcan_security_attr.lpSecurityDescriptor = &pcan_security_desc;
    pcan_security_attr.bInheritHandle = true;

    /* Create event object that worker thread will use to communicate
       CAN read events with main thread */
    pcan_read_event = CreateEvent(&pcan_security_attr, /* lpEventAttributes */
				  false, /* bManualReset */
				  false, /* bInitialState */
				  "pcan_read_event");

    if (pcan_read_event == 0) {
	printf("pcan_EnableEvent: Error at CreateEvent (pcan_read_event) (0x%02X)",
	       GetLastError());
	return 1;
    }

    /* Create event object that main thread will use to communicate with the 
       worker thread */
    pcan_thread_event = CreateEvent(&pcan_security_attr, /* lpEventAttributes */
				    false, /* bManualReset */
				    false, /* bInitialState */
				    "pcan_thread_event");

    if (pcan_thread_event == 0) {
	printf("pcan_EnableEvent: Error at CreateEvent (pcan_thread_event) (0x%02X)",
	       GetLastError());
	return 1;
    }

    /* Spawn the worker thread that will monitor the event */
    pcan_thread = CreateThread(&pcan_security_attr, /* lpThreadAttributes */
			       0, /* dwStackSize */
			       &pcan_thread_proc, /* lpStartAddress */
			       (void*)pcan_Channel, /* lpParameter */
			       0, /* dwCreationFlags */
			       &pcan_thread_id); /* lpThreadId */

    if (pcan_thread == 0) {
	printf("pcan_EnableEvent: Error at CreateThread (0x%02X)", GetLastError());
	return 1;
    }

    /* Wait for pcan_thread_event to be set, indicating that the thread
       was successfully started */
    printf("pcan_EnableEvent: Waiting for worker thread to start...\n");

    ret = WaitForSingleObject(pcan_thread_event, 1000);
    switch (ret) {
    case WAIT_OBJECT_0:
	break;
    case WAIT_ABANDONED:
	printf("pcan_EnableEvent: Error at WaitForSingleObject: WAIT_ABANDONDED\n");
	return 1;
    case WAIT_TIMEOUT:
	printf("pcan_EnableEvent: Error at WaitForSingleObject: WAIT_TIMEOUT\n");
	return 1;
    case WAIT_FAILED:
	printf("pcan_EnableEvent: Error at WaitForSingleObject: WAIT_FAILED (0x%02X)\n",
	       GetLastError());
	return 1;
    default:
	printf("pcan_EnableEvent: Error at WaitForSingleObject: ret = 0x%02X\n", ret);
	return 1;
    }

    /* Set the PCAN library to use the receive event */
    TPCANStatus pcan_result = 0;

    pcan_result = CAN_SetValue(pcan_Channel, PCAN_RECEIVE_EVENT,
			       &pcan_read_event, sizeof(pcan_read_event));
    
    if (pcan_result != PCAN_ERROR_OK) {
	printf("pcan_EnableEvent: Error at CAN_SetValue: 0x%02X (%s)\n",
	       pcan_result, pcan_status_lookup(pcan_result));
	return 1;
    }

    return 0;
}




int pcan_DisableEvent(TPCANHandle pcan_Channel)
{
    int ret = -1;

    /* Remove event */
    HANDLE dummy_event = INVALID_HANDLE_VALUE;
    TPCANStatus pcan_result = 0;

    pcan_result = CAN_SetValue(pcan_Channel, PCAN_RECEIVE_EVENT,
			       &dummy_event, sizeof(dummy_event));

    /* Stop the thread */  
    ret = SetEvent(pcan_thread_event);
    if (ret == 0) {
	printf("pcan_DisableEvent: Error at SetEvent (0x%02X)\n",
	       GetLastError());
	return 1;
    }

    /* Wait for the thread to exit */
    int pcan_thread_exit_code = 0;
    
    printf("pcan_DisableEvent: Waiting for worker thread to exit...\n");
    ret = WaitForSingleObject(pcan_thread, INFINITE);

    switch (ret) {
    case WAIT_OBJECT_0:
	ret = GetExitCodeThread(pcan_thread, &pcan_thread_exit_code);

	if (ret == 0) {
	    printf("pcan_DisableEvent: Error at GetExitCodeThread: 0x%02X\n",
		   GetLastError());
	    return 1;
	}

	if (pcan_thread_exit_code == STILL_ACTIVE) {
	    printf("pcan_DisableEvent: Thread still active.\n");
	    return 1;
	} else {
	    printf("pcan_DisableEvent: Worker thread exited with code %i\n",
		   pcan_thread_exit_code);
	}
	break;
    case WAIT_ABANDONED:
	printf("pcan_DisableEvent: Error at WaitForSingleObject: WAIT_ABANDONDED\n");
	return 1;
    case WAIT_TIMEOUT:
	printf("pcan_DisableEvent: Error at WaitForSingleObject: WAIT_TIMEOUT\n");
	return 1;
    case WAIT_FAILED:
	printf("pcan_DisableEvent: Error at WaitForSingleObject: WAIT_FAILED (0x%02X)\n",
	       GetLastError());
	return 1;
    default:
	printf("pcan_DisableEvent: Error at WaitForSingleObject: ret = 0x%02X\n", ret);
	return 1;
    }

    /* Close all handles */
    ret = CloseHandle(pcan_read_event);
    if (ret == 0) {
	printf("pcan_DisableEvent: Error at CloseHandle(pcan_read_event): %i\n",
	       GetLastError());
	return 1;
    }

    ret = CloseHandle(pcan_thread_event);
    if (ret == 0) {
	printf("pcan_DisableEvent: Error at CloseHandle(pcan_thread_event): %i\n",
	       GetLastError());
	return 1;
    }

    ret = CloseHandle(pcan_thread);
    if (ret == 0) {
	printf("pcan_DisableEvent: Error at CloseHandle(pcan_thread): %i\n",
	       GetLastError());
	return 1;
    }

    return 0;
}

