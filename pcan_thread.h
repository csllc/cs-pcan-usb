#ifndef _PCAN_THREAD_H_
#define _PCAN_THREAD_H_

#include <windows.h>

#define EVENT_INDEX_READ (0)
#define EVENT_INDEX_THREAD (1)

typedef struct threadParameters_s {
    int canChannel;
    void (*callback) (int);
} threadParameters_t;
    
DWORD WINAPI pcan_thread_proc(LPVOID lpParam);
//void __stdcall pcan_thread_proc(LPVOID lpParam);
void pcan_thread_read(int channel);

#endif /* _PCAN_THREAD_H_ */
