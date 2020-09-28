/* N-API interface for PEAK-System PCAN-Basic

   N-API callable functions that directly correspond to PCAN-Basic functions, 
   and additional wrapper functions

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/
   

#include <assert.h>
#include <node_api.h> // provide N-API types, constants, and functions
#include <stdio.h>    // provide printf
#include <windows.h>  // provide Win32 types, constants, and functions
//#include <process.h>  // ??

#include "PCANBasic.h"

#include "pcan.h"
#include "pcan_helper.h"
#include "pcan_event.h"
#include "napi_helper.h"


// ----------------------------------- // -----------------------------------
// Definitions


// Length of N-API module descriptor list, used in Init
enum { descriptors_len = 17 };




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables

// Data read callback function, created in main thread and called from worker
// thread
napi_threadsafe_function pcan_callback = { 0 };




// ----------------------------------- // -----------------------------------
// Local functions


// Register N-API module
NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);




napi_value Init(napi_env env, napi_value exports)
{
    napi_status status = napi_generic_failure;

    // Turn off buffering on stdout
    setvbuf(stdout, NULL, _IONBF, 0);

    // Build array of property descriptors
    napi_property_descriptor descriptors[descriptors_len] = {
	// PCAN-Basic functions
	DECLARE_NAPI_METHOD("CAN_Initialize", pcan_CAN_Initialize),
	DECLARE_NAPI_METHOD("CAN_InitializeFD", pcan_CAN_InitializeFD),
	DECLARE_NAPI_METHOD("CAN_Uninitialize", pcan_CAN_Uninitialize),
	DECLARE_NAPI_METHOD("CAN_Reset", pcan_CAN_Reset),
	DECLARE_NAPI_METHOD("CAN_GetStatus", pcan_CAN_GetStatus),
	DECLARE_NAPI_METHOD("CAN_Read", pcan_CAN_Read),
	DECLARE_NAPI_METHOD("CAN_ReadFD", pcan_CAN_ReadFD),
	DECLARE_NAPI_METHOD("CAN_Write", pcan_CAN_Write),
	DECLARE_NAPI_METHOD("CAN_WriteFD", pcan_CAN_Write),
	DECLARE_NAPI_METHOD("CAN_GetValue", pcan_CAN_GetValue),
	DECLARE_NAPI_METHOD("CAN_SetValue", pcan_CAN_SetValue),
	DECLARE_NAPI_METHOD("CAN_FilterMessages", pcan_CAN_FilterMessages),
	DECLARE_NAPI_METHOD("CAN_GetErrorText", pcan_CAN_GetErrorText),
	// Additional functions
	DECLARE_NAPI_METHOD("CAN_EnableEvent", pcan_CAN_EnableEvent),
	DECLARE_NAPI_METHOD("CAN_DisableEvent", pcan_CAN_DisableEvent),
	DECLARE_NAPI_METHOD("CAN_AcceptanceFilter29Bit", pcan_CAN_AcceptanceFilter29Bit),
	DECLARE_NAPI_METHOD("CAN_AcceptanceFilter11Bit", pcan_CAN_AcceptanceFilter11Bit),
    };

    status = napi_define_properties(env, exports, descriptors_len, descriptors);
    assert(status == napi_ok);

    return exports;
}




void pcan_CAN_EventCallback(int channel)
{
    napi_status status = napi_generic_failure;

#ifdef PCAN_DEBUG
    printf("pcan_CAN_EventCallback()\n");
#endif

    status = napi_call_threadsafe_function(pcan_callback, 0, true);
    assert(status == napi_ok);

    return;
}




// ----------------------------------- // -----------------------------------
// Public functions


napi_value pcan_CAN_Initialize(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_INITIALIZE_ARGC;
    napi_value argv[CAN_INITIALIZE_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_INITIALIZE_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments."
			      "This implemtnation of CAN_Initialize accepts Channel and "
			      "Btr0Btr1.");
	return 0;
    }

    // Retreive arguments
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    uint32_t pcanBtr0Btr1;
    status = napi_get_value_int32(env, argv[1], (int32_t*)&pcanBtr0Btr1);
    assert(status == napi_ok);

    // Initialize CAN bus
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_Initialize(pcanChannel, pcanBtr0Btr1, 0, 0, 0);
    
    // Print results to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_Initialize:\n"
	   "  pcanStatus = 0x%02X (%s)\n"
	   "  pcanChannel = 0x%02X\n"
	   "  pcanBtr0Btr1 = 0x%02X\n",
	   pcanStatus, pcanStatusLookup(pcanStatus),
	   pcanChannel,
	   pcanBtr0Btr1);
#endif
    
    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_InitializeFD(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argment list
    size_t argc = CAN_INITIALIZEFD_ARGC;
    napi_value argv[CAN_INITIALIZEFD_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_INITIALIZEFD_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retreive arguments
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    char pcanBitrateFD[256] = { 0 };
    size_t pcanBitrateFDResult = 0;
    status = napi_get_value_string_utf8(env, argv[1], pcanBitrateFD,
					sizeof(pcanBitrateFD), &pcanBitrateFDResult);
    assert(status == napi_ok);

    // Initialize CAN bus, with flexible data rate capabilities
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_InitializeFD(pcanChannel, pcanBitrateFD);
    
    // Print results to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_InitializeFD:\n"
	   "  pcanStatus = 0x%02X (%s)\n"
	   "  pcanChannel = 0x%02X\n"
	   "  pcanBitrateFD = \"%s\"\n",
	   pcanStatus, pcanStatusLookup(pcanStatus),
	   pcanChannel,
	   pcanBitrateFD);
#endif

    // Create N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_Uninitialize(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_UNINITIALIZE_ARGC;
    napi_value argv[CAN_UNINITIALIZE_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_UNINITIALIZE_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // Uninitialize CAN bus
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_Uninitialize(pcanChannel);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_Uninitialize: 0x%02X (%s)\n",
	   pcanStatus, pcanStatusLookup(pcanStatus));
#endif

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_Reset(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_RESET_ARGC;
    napi_value argv[CAN_RESET_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_RESET_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // Reset CAN bus
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_Reset(pcanChannel);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_Reset: 0x%02X (%s)\n",
	   pcanStatus, pcanStatusLookup(pcanStatus));
#endif

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_GetStatus(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_GETSTATUS_ARGC;
    napi_value argv[CAN_GETSTATUS_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_GETSTATUS_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // Get CAN status
    TPCANStatus pcan_can_status;
    pcan_can_status = CAN_GetStatus(pcanChannel);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_GetStatus: 0x%02X (%s)\n",
	   pcan_can_status, pcanStatusLookup(pcan_can_status));
#endif

    // Create a N-API value for the status and return it
    napi_value can_status;
    status = napi_create_bigint_uint64(env, pcan_can_status, &can_status);
    assert(status == napi_ok);

    return can_status;
}




napi_value pcan_CAN_Read(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and arguments list
    size_t argc = CAN_READ_ARGC;
    napi_value argv[CAN_READ_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_READ_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // argv[1] MessageBuffer
    napi_valuetype pcanMessageBufferType;
    status = napi_typeof(env, argv[1], &pcanMessageBufferType);
    assert(status == napi_ok);

    if (pcanMessageBufferType != napi_object)
    {
	napi_throw_type_error(env, 0, "Argument 1 (MessageBuffer) is not an object.");
	return 0;
    }

    // argv[2] TimestampBuffer
    napi_valuetype pcanTimestampBufferType;
    status = napi_typeof(env, argv[2], &pcanTimestampBufferType);
    assert(status == napi_ok);

    if (pcanTimestampBufferType != napi_object)
    {
	napi_throw_type_error(env, 0, "Argument 2 (TimestampBuffer) is not an object.");
	return 0;
    }

    // Read from CAN bus
    TPCANMsg msg = { 0 };
    TPCANTimestamp timestamp = { 0 };
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;

    pcanStatus = CAN_Read(pcanChannel, &msg, &timestamp);
    
    // Print the result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_Read: 0x%02X (%s)\n", pcanStatus, pcanStatusLookup(pcanStatus));
    pcanDumpMsg(&msg);
    pcanDumpTimestamp(&timestamp);
#endif

    // Copy read values into objects passed from N-API
    // arvg[1] MessageBuffer
    napi_value pcanMessageBufferID; // uint32
    napi_value pcanMessageBufferMsgtype; // uint32
    napi_value pcanMessageBufferLen; // uint32
    napi_value pcanMessageBufferData; // buffer
    void* pcanMessageBufferDataResult;
    
    status = napi_create_uint32(env, msg.ID, &pcanMessageBufferID);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "id", pcanMessageBufferID);
    assert(status == napi_ok);
    
    status = napi_create_uint32(env, msg.MSGTYPE, &pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "msgtype", pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    
    status = napi_create_uint32(env, msg.LEN, &pcanMessageBufferLen);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "len", pcanMessageBufferLen);
    assert(status == napi_ok);

    status = napi_create_buffer_copy(env, msg.LEN, (void*)&(msg.DATA),
				     (void*)&pcanMessageBufferDataResult,
				     &pcanMessageBufferData);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "data", pcanMessageBufferData);
    assert(status == napi_ok);

    // argv[2] TimestampBuffer
    napi_value pcanTimestampBufferMillis; // uint32
    napi_value pcanTimestampBufferMillisOverflow; // uint32
    napi_value pcanTimestampBufferMicros; // uint32

    status = napi_create_uint32(env, timestamp.millis, &pcanTimestampBufferMillis);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[2], "millis",
				     pcanTimestampBufferMillis);

    status = napi_create_uint32(env, timestamp.millis_overflow,
				&pcanTimestampBufferMillisOverflow);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[2], "millis_overflow",
				     pcanTimestampBufferMillisOverflow);

    status = napi_create_uint32(env, timestamp.micros, &pcanTimestampBufferMicros);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[2], "micros",
				     pcanTimestampBufferMicros);

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);
    return result;
}




napi_value pcan_CAN_ReadFD(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_READFD_ARGC;
    napi_value argv[CAN_READFD_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_READFD_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // argv[1] MessageBuffer
    napi_valuetype pcanMessageBufferType;
    status = napi_typeof(env, argv[1], &pcanMessageBufferType);
    assert(status == napi_ok);

    if (pcanMessageBufferType != napi_object)
    {
	napi_throw_type_error(env, 0, "Argument 1 (MessageBufferFD) is not an object.");
	return 0;
    }

    // argv[2] TimestampBuffer
    napi_valuetype pcanTimestampBufferType;
    status = napi_typeof(env, argv[2], &pcanTimestampBufferType);
    assert(status == napi_ok);

    if (pcanTimestampBufferType != napi_object)
    {
	napi_throw_type_error(env, 0, "Argument 2 (TimestampBufferFD) is not an object.");
	return 0;
    }

    // Read from CAN bus
    TPCANMsgFD msg = { 0 };
    TPCANTimestampFD timestamp = 0;
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;

    pcanStatus = CAN_ReadFD(pcanChannel, &msg, &timestamp);

    // Dump results to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_ReadFD: 0x%02X (%s)\n", pcanStatus,
	   pcanStatusLookup(pcanStatus));
    pcanDumpMsgFD(&msg);
    pcanDumpTimestampFD(timestamp);
#endif

    // Copy read values into objects passed from N-API
    // arvg[1] MessageBuffer
    napi_value pcanMessageBufferID; // uint32
    napi_value pcanMessageBufferMsgtype; // uint32
    napi_value pcanMessageBufferDLC; // uint32
    napi_value pcanMessageBufferData; // buffer
    void* pcanMessageBufferDataResult;
    
    status = napi_create_uint32(env, msg.ID, &pcanMessageBufferID);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "id", pcanMessageBufferID);
    assert(status == napi_ok);
    
    status = napi_create_uint32(env, msg.MSGTYPE, &pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "msgtype", pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    
    status = napi_create_uint32(env, msg.DLC, &pcanMessageBufferDLC);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "dlc", pcanMessageBufferDLC);
    assert(status == napi_ok);

    status = napi_create_buffer_copy(env, pcanDLCDecode(msg.DLC), (void*)&(msg.DATA), (void*)&pcanMessageBufferDataResult, &pcanMessageBufferData);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "data", pcanMessageBufferData);
    assert(status == napi_ok);

    // argv[2] TimestampFD
    napi_value pcanTimestampFDValue; // uint64

    status = napi_create_uint32(env, timestamp, &pcanTimestampFDValue);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[2], "timestamp", pcanTimestampFDValue);
    assert(status == napi_ok);

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_Write(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve calback info and argument list
    size_t argc = CAN_WRITE_ARGC;
    napi_value argv[CAN_WRITE_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_WRITE_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // argv[1] MessageBuffer
    napi_valuetype pcanMessageBufferType;
    status = napi_typeof(env, argv[1], &pcanMessageBufferType);
    assert(status == napi_ok);

    if (pcanMessageBufferType != napi_object)
    {
	napi_throw_type_error(env, 0, "Argument 1 (MessageBuffer) is not an object.");
	return 0;
    }

    // Copy message contents from N-API object to TPCANMsg struct
    TPCANMsg msg = { 0 };

    napi_value pcanMessageBufferID;
    status = napi_get_named_property(env, argv[1], "id", &pcanMessageBufferID);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferID, &(msg.ID));
    assert(status == napi_ok);

    napi_value pcanMessageBufferMsgtype;
    status = napi_get_named_property(env, argv[1], "msgtype", &pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferMsgtype, (uint32_t*)&(msg.MSGTYPE));
    assert(status == napi_ok);

    napi_value pcanMessageBufferLen;
    status = napi_get_named_property(env, argv[1], "len", &pcanMessageBufferLen);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferLen, (uint32_t*)&(msg.LEN));
    assert(status == napi_ok);

    napi_value pcanMessageBufferData;
    BYTE *msgTmpData;
    size_t msgTmpLen = 0;
    status = napi_get_named_property(env, argv[1], "data", &pcanMessageBufferData);
    assert(status == napi_ok);
    status = napi_get_buffer_info(env, pcanMessageBufferData, (void**)&msgTmpData, &msgTmpLen);
    assert(status == napi_ok);

    if (msgTmpLen != msg.LEN)
    {
	napi_throw_error(env, 0, "Mismatch between LEN and actual data buffer length");
	return 0;
    }

    memcpy(msg.DATA, msgTmpData, msg.LEN);

    // Write to CAN bus
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_Write(pcanChannel, &msg);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_Write: 0x%02X (%s)\n", pcanStatus, pcanStatusLookup(pcanStatus));
    pcanDumpMsg(&msg);
#endif

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);
    
    return result;
}




napi_value pcan_CAN_WriteFD(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Get calback info and argument list
    size_t argc = CAN_WRITEFD_ARGC;
    napi_value argv[CAN_WRITEFD_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_WRITEFD_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // argv[1] MessageBufferFD
    napi_valuetype pcanMessageBufferType;
    status = napi_typeof(env, argv[1], &pcanMessageBufferType);
    assert(status == napi_ok);

    if (pcanMessageBufferType != napi_object)
    {
	napi_throw_type_error(env, 0, "Argument 1 (MessageBufferFD) is not an object.");
	return 0;
    }

    // Copy message contents from N-API object to TPCANMsgFD struct
    TPCANMsgFD msg = { 0 };

    napi_value pcanMessageBufferID;
    status = napi_get_named_property(env, argv[1], "id", &pcanMessageBufferID);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferID, &(msg.ID));
    assert(status == napi_ok);

    napi_value pcanMessageBufferMsgtype;
    status = napi_get_named_property(env, argv[1], "msgtype", &pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferMsgtype, (uint32_t*)&(msg.MSGTYPE));
    assert(status == napi_ok);

    napi_value pcanMessageBufferDLC;
    status = napi_get_named_property(env, argv[1], "dlc", &pcanMessageBufferDLC);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferDLC, (uint32_t*)&(msg.DLC));
    assert(status == napi_ok);

    napi_value pcanMessageBufferData;
    BYTE *msgTmpData;
    size_t msgTmpLen = 0;
    status = napi_get_named_property(env, argv[1], "data", &pcanMessageBufferData);
    assert(status == napi_ok);
    status = napi_get_buffer_info(env, pcanMessageBufferData, (void**)&msgTmpData, &msgTmpLen);
    assert(status == napi_ok);

    if (msgTmpLen != pcanDLCDecode(msg.DLC))
    {
	napi_throw_error(env, 0, "Mismatch between DLC and actual data buffer length");
	return 0;
    }

    memcpy(msg.DATA, msgTmpData, pcanDLCDecode(msg.DLC));

    // Write to CAN bus
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_WriteFD(pcanChannel, &msg);
    
    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_WriteFD: 0x%02X (%s)\n", pcanStatus, pcanStatusLookup(pcanStatus));
    pcanDumpMsgFD(&msg);
#endif

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_GetValue(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_GETVALUE_ARGC;
    napi_value argv[CAN_GETVALUE_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_GETVALUE_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments. \n"
			      "The N-API implementation of CAN_GetValue accepts Channel, "
			      "Parameter, and Buffer.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // argv[1] Parameter
    uint32_t pcanParameter;
    status = napi_get_value_int32(env, argv[1], (int32_t*)&pcanParameter);
    assert(status == napi_ok);

    // argv[2] Buffer
    napi_valuetype pcanBufferType;
    status = napi_typeof(env, argv[2], &pcanBufferType);
    assert(status == napi_ok);

    if (pcanBufferType != napi_object)
    {
	napi_throw_type_error(env, 0, "Argument 2 (Buffer) is not an object.");
	return 0;
    }

    // Argument 3 - BufferLength (obtained from pcanBuffer)
    BYTE *pcanBuffer = 0;
    size_t pcanBufferLength = 0;
    status = napi_get_buffer_info(env, argv[2], (void**)&pcanBuffer, &pcanBufferLength);
    assert(status == napi_ok);

    // Get requested parameter from CAN channel
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_GetValue(pcanChannel, pcanParameter, pcanBuffer, pcanBufferLength);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_GetValue: \n"
	   "  pcanStatus       = 0x%02X (%s)\n"
	   "  pcanChannel      = 0x%02X\n"
	   "  pcanParameter    = 0x%02X (%s)\n"
	   "  pcanBufferLength = 0x%02zX\n"
	   "  pcanBuffer       = ",
	   pcanStatus, pcanStatusLookup(pcanStatus),
	   pcanChannel,
	   pcanParameter, pcanParameterLookup(pcanParameter),
	   pcanBufferLength);
    pcanDumpBuffer(pcanBuffer, pcanBufferLength);
    printf("\n");
#endif

    // Put value into the buffer object passed from N-API
    napi_value pcanBufferValue;
    status = napi_create_buffer(env, pcanBufferLength, &pcanBuffer, &pcanBufferValue);
    assert(status == napi_ok);

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_SetValue(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_SETVALUE_ARGC;
    napi_value argv[CAN_SETVALUE_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_SETVALUE_ARGC)
    {
	napi_throw_type_error(env, 0, "Incorrect number of arguments. \n"
			      "The N-API implementation of CAN_SetValue accepts Channel, "
			      "Parameter, and Buffer.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // argv[1] Parameter
    uint32_t pcanParameter;
    status = napi_get_value_int32(env, argv[1], (int32_t*)&pcanParameter);
    assert(status == napi_ok);

    // argv[2] Buffer
    napi_valuetype pcanBufferType;
    status = napi_typeof(env, argv[2], &pcanBufferType);
    assert(status == napi_ok);

    if (pcanBufferType != napi_object)
    {
	napi_throw_type_error(env, 0, "Argument 2 (Buffer) is not an object.");
	return 0;
    }

    // Argument 3 - BufferLength (obtained from pcanBuffer)
    BYTE *pcanBuffer = 0;
    size_t pcanBufferLength = 0;
    
    status = napi_get_buffer_info(env, argv[2], (void**)&pcanBuffer, &pcanBufferLength);
    assert(status == napi_ok);

    // Set value for CAN channel
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_SetValue(pcanChannel, pcanParameter, pcanBuffer, pcanBufferLength);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_SetValue: \n"
	   "  pcanStatus       = 0x%02X (%s)\n"
	   "  pcanChannel      = 0x%02X\n"
	   "  pcanParameter    = 0x%02X (%s)\n"
	   "  pcanBufferLength = 0x%02zX\n"
	   "  pcanBuffer       = ",
	   pcanStatus, pcanStatusLookup(pcanStatus),
	   pcanChannel,
	   pcanParameter, pcanParameterLookup(pcanParameter),
	   pcanBufferLength);
    pcanDumpBuffer(pcanBuffer, pcanBufferLength);
    printf("\n");
#endif

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_FilterMessages(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_FILTERMESSAGES_ARGC;
    napi_value argv[CAN_FILTERMESSAGES_ARGC];
    
    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_FILTERMESSAGES_ARGC) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // argv[1] FromID
    uint32_t pcanFromID;
    status = napi_get_value_int32(env, argv[1], (int32_t*)&pcanFromID);
    assert(status == napi_ok);

    // argv[2] ToID
    uint32_t pcanToID;
    status = napi_get_value_int32(env, argv[2], (int32_t*)&pcanToID);
    assert(status == napi_ok);

    // argv[3] Mode
    uint32_t pcanMode;
    status = napi_get_value_int32(env, argv[3], (int32_t*)&pcanMode);
    assert(status == napi_ok);

    // Set or ammend the CAN message filter ranges
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_FilterMessages(pcanChannel, pcanFromID, pcanToID, pcanMode);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_FilterMessages: \n"
	   "  pcanStatus  = 0x%02X (%s)\n"
	   "  pcanChannel = 0x%02X\n"
	   "  pcanFromID  = 0x%08X\n"
	   "  pcanToID    = 0x%08X\n"
	   "  pcanMode    = 0x%02X\n",
	   pcanStatus, pcanStatusLookup(pcanStatus),
	   pcanChannel,
	   pcanFromID,
	   pcanToID,
	   pcanMode);
#endif

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_GetErrorText(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;
    
    // Retrieve callback info and argument list
    size_t argc = CAN_GETERRORTEXT_ARGC;
    napi_value argv[CAN_GETERRORTEXT_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_GETERRORTEXT_ARGC) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Error
    TPCANStatus pcan_Error;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcan_Error);
    assert(status == napi_ok);

    // argv[1] Language
    uint32_t pcan_Language;
    status = napi_get_value_int32(env, argv[1], (int32_t*)&pcan_Language);
    assert(status == napi_ok);

    // argv[2] Buffer
    char pcanBuffer[256];

    // Call API to get error text
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_GetErrorText(pcan_Error, pcan_Language, pcanBuffer);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_GetErrorText:\n"
	   "  pcanStatus    = %i (%s)\n"
	   "  pcan_Error    = %i\n"
	   "  pcan_Language = %i\n"
	   "  pcanBuffer    = \"%s\"\n",
	   pcanStatus, pcanStatusLookup(pcanStatus),
	   pcan_Error,
	   pcan_Language,
	   pcanBuffer);
#endif

    // Copy result into 'buffer' property of object passed from N-API
    napi_value pcanBuffer_value;

    status = napi_create_string_utf8(env, pcanBuffer, NAPI_AUTO_LENGTH, &pcanBuffer_value);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[2], "buffer", pcanBuffer_value);
    assert(status == napi_ok);
    
    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_EnableEvent(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Get callback info and argument list
    size_t argc = CAN_ENABLEEVENT_ARGC;
    napi_value argv[CAN_ENABLEEVENT_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_ENABLEEVENT_ARGC) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // argv[1] Callback
    napi_valuetype pcan_callback_type;
    status = napi_typeof(env, argv[1], &pcan_callback_type);
    assert(status == napi_ok);

    // Create resource name string, used below
    napi_value async_resource_name;
    status = napi_create_string_utf8(env, "pcan_callback",
				     NAPI_AUTO_LENGTH, &async_resource_name);
    assert(status == napi_ok);

    // Create thread-safe function to use as a callback
    status = napi_create_threadsafe_function(env,
					     argv[1],
					     0, // async_resource
					     async_resource_name,
					     0, // max_queue_size
					     1, // initial_thread_count
					     0, // thread_finalize_data
					     0, // thread_finalize_cb
					     0, // context
					     0, // call_js_cb
					     &pcan_callback); // Local variable
    assert(status == napi_ok);

    // Enable CAN Bus receive event
    int pcanStatus;
    pcanStatus = pcanEventEnable(pcanChannel, pcan_CAN_EventCallback);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_EnableEvent: 0x%02X\n", pcanStatus);
#endif

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_DisableEvent(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    /// Get callback info and argument list
    size_t argc = CAN_DISABLEEVENT_ARGC;
    napi_value argv[CAN_DISABLEEVENT_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_DISABLEEVENT_ARGC) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // Disable CAN Bus receive event
    int pcanStatus;
    pcanStatus = pcanEventDisable(pcanChannel);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_DisableEvent: 0x%02X\n", pcanStatus);
#endif

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_AcceptanceFilter11Bit(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Get callback info and argument list
    size_t argc = CAN_ACCEPTANCEFILTER11BIT_ARGC;
    napi_value argv[CAN_ACCEPTANCEFILTER11BIT_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_ACCEPTANCEFILTER11BIT_ARGC) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // argv[1] AcceptanceCode
    uint32_t pcan_AcceptanceCode;
    status = napi_get_value_int32(env, argv[1], (int32_t*)&pcan_AcceptanceCode);
    assert(status == napi_ok);

    // argv[2] AcceptanceMask
    uint32_t pcan_AcceptanceMask;
    status = napi_get_value_int32(env, argv[2], (int32_t*)&pcan_AcceptanceMask);
    assert(status == napi_ok);

    // Build 64-bit acceptance filter value
    uint64_t pcan_AcceptanceFilter = 0;
    pcan_AcceptanceFilter |= ((uint64_t)pcan_AcceptanceCode << 0x20);
    pcan_AcceptanceFilter |= ((uint64_t)(~pcan_AcceptanceMask));
    // Note that pcan_AcceptanceMask's bits are flipped from "don't care"
    // to "do care"

    // Set the acceptance filter through the API
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_SetValue(pcanChannel, PCAN_ACCEPTANCE_FILTER_11BIT,
			       &pcan_AcceptanceFilter, sizeof(pcan_AcceptanceFilter));

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_AcceptanceFilter11Bit:\n"
	   "  pcanStatus            = 0x%02X (%s)\n"
	   "  pcanChannel           = 0x%02X\n"
	   "  pcan_AcceptanceCode   = 0x%08X\n"
	   "  pcan_AcceptanceMask   = 0x%08X\n"
	   "  pcan_AcceptanceFilter = 0x%016llX\n",
	   pcanStatus, pcanStatusLookup(pcanStatus),
	   pcanChannel,
	   pcan_AcceptanceCode,
	   pcan_AcceptanceMask,
	   pcan_AcceptanceFilter);
#endif

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;    
}




napi_value pcan_CAN_AcceptanceFilter29Bit(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Get callback info and argument list
    size_t argc = CAN_ACCEPTANCEFILTER29BIT_ARGC;
    napi_value argv[CAN_ACCEPTANCEFILTER29BIT_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_ACCEPTANCEFILTER29BIT_ARGC) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_int32(env, argv[0], (int32_t*)&pcanChannel);
    assert(status == napi_ok);

    // argv[1] AcceptanceCode
    uint32_t pcan_AcceptanceCode;
    status = napi_get_value_int32(env, argv[1], (int32_t*)&pcan_AcceptanceCode);
    assert(status == napi_ok);

    // argv[2] AcceptanceMask
    uint32_t pcan_AcceptanceMask; /* TODO: use uint32_t or uint64_t universally where appropriate, and cast inside napi_get_value_int32 call */
    status = napi_get_value_int32(env, argv[2], (int32_t*)&pcan_AcceptanceMask);
    assert(status == napi_ok);

    // Build 64-bit acceptance filter value
    uint64_t pcan_AcceptanceFilter = 0;
    pcan_AcceptanceFilter |= ((uint64_t)pcan_AcceptanceCode << 0x20);
    pcan_AcceptanceFilter |= ((uint64_t)(~pcan_AcceptanceMask));
    // Note that pcan_AcceptanceMask's bits are flipped from "don't care"
    // to "do care"

    // Set the acceptance filter through the API
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_SetValue(pcanChannel, PCAN_ACCEPTANCE_FILTER_29BIT,
			       &pcan_AcceptanceFilter, sizeof(pcan_AcceptanceFilter));

// Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_AcceptanceFilter29Bit:\n"
	   "  pcanStatus            = 0x%02X (%s)\n"
	   "  pcanChannel           = 0x%02X\n"
	   "  pcan_AcceptanceCode   = 0x%08X\n"
	   "  pcan_AcceptanceMask   = 0x%08X\n"
	   "  pcan_AcceptanceFilter = 0x%016llX\n",
	   pcanStatus, pcanStatusLookup(pcanStatus),
	   pcanChannel,
	   pcan_AcceptanceCode,
	   pcan_AcceptanceMask,
	   pcan_AcceptanceFilter);
#endif

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_int32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;    
}


