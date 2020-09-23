#ifndef _PCAN_THREAD_H_
#define _PCAN_THREAD_H_

#include <windows.h>

#define EVENT_INDEX_READ (0)
#define EVENT_INDEX_THREAD (1)

typedef union thread_data_u {
    /* Array of handles used by WaitForMultipleObjects */
    HANDLE events[2];

    /* Individual handles for different events */
    struct thread_data_s {
	/*HANDLE read_event;
	  HANDLE thread_event;*/
	int can_channel;
    };
} thread_data_t;
    
DWORD WINAPI pcan_thread_proc(LPVOID lpParam);
//void __stdcall pcan_thread_proc(LPVOID lpParam);
void pcan_thread_read(int channel);

#endif /* _PCAN_THREAD_H_ */
