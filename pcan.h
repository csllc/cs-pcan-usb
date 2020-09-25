#ifndef _PCAN_H_
#define _PCAN_H_

// Uncomment the following to disable declarations for functions that use N-API.
// This is required when compiling test programs that rely soley on the Win32
// and PCAN-Basic APIs.

//#define _PCAN_WIN32_ONLY_
#ifndef _PCAN_WIN32_ONLY_
#include <node_api.h>

// pcan_CAN_Initialize
// Arguments passed through N-API:
// TPCANHandle Channel (int32)
// TPCANBaudrate Btr0Btr1 (int32)

#define CAN_INITIALIZE_ARGC (2)

napi_value pcan_CAN_Initialize(napi_env env, napi_callback_info info);

#define CAN_INITIALIZEFD_ARGC (2)


#endif _PCAN_WIN32_ONLY_


void pcan_CAN_EventCallback(int channel);

#endif /* _PCAN_H_ */
