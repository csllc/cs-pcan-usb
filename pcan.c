#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include <windows.h>

#include "PCANBasic.h"

#include "helper.h"



napi_value pcan_CAN_Initialize(napi_env env, napi_callback_info info)
{
    napi_status status;

    /* Get callback info and argments: 
       TPCANHandle Channel (int32)
       TPCANBaudrate Btr0Btr1 (int32) */
    size_t argc = 2;
    napi_value argv[2];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != 2) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    /* Retreive arguments */
    int32_t pcan_Channel; /* TODO: Should we just be using uint64_t for everything? */
    status = napi_get_value_int32(env, argv[0], &pcan_Channel);
    assert(status == napi_ok);
    printf("pcan_CAN_Initialize: pcan_Channel = 0x%02X\n", pcan_Channel);

    int32_t pcan_Btr0Btr1;
    status = napi_get_value_int32(env, argv[1], &pcan_Btr0Btr1);
    assert(status == napi_ok);
    printf("pcan_CAN_Initialize: pcan_Btr0Btr1 = 0x%02X\n", pcan_Btr0Btr1);

    /* Initialize CAN bus */
    TPCANStatus pcan_result;

    pcan_result = CAN_Initialize(pcan_Channel, pcan_Btr0Btr1, 0, 0, 0);
    printf("pcan_CAN_Initialize: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));
    
    /* Create N-API value for the result and return it */
    napi_value result;
    status = napi_create_int32(env, pcan_result, &result);

    return result;

}




napi_value pcan_CAN_Uninitialize(napi_env env, napi_callback_info info)
{
    napi_status status;

    /* Get callback info and arguments:
       TPCANHandle Channel (int32) */
    size_t argc = 1;
    napi_value argv[1];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != 1) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    /* Retrieve arguments */
    int32_t pcan_Channel;
    status = napi_get_value_int32(env, argv[0], &pcan_Channel);
    assert(status == napi_ok);

    /* Uninitialize CAN bus */
    TPCANStatus pcan_result;

    pcan_result = CAN_Uninitialize(pcan_Channel);
    printf("pcan_CAN_Uninitialize: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));

    /* Create a N-API value for the result and return it */
    napi_value result;
    status = napi_create_int32(env, pcan_result, &result);

    return result;
}




napi_value pcan_CAN_Reset(napi_env env, napi_callback_info info)
{
    napi_status status;

    /* Get callback info and arguments:
       TPCANHandle Channel (int32) */
    size_t argc = 1;
    napi_value argv[1];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != 1) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    /* Retrieve arguments */
    int32_t pcan_Channel;
    status = napi_get_value_int32(env, argv[0], &pcan_Channel);
    assert(status == napi_ok);

    /* Reset CAN bus */
    TPCANStatus pcan_result;

    pcan_result = CAN_Reset(pcan_Channel);
    printf("pcan_CAN_Reset: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));

    /* Create a N-API value for the result and return it */
    napi_value result;
    status = napi_create_int32(env, pcan_result, &result);

    return result;
}




napi_value pcan_CAN_GetStatus(napi_env env, napi_callback_info info)
{
    napi_status status;

    /* Get callback info and arguments:
       TPCANHandle Channel (int32) */
    size_t argc = 1; /* TODO: make these constants somewhere, instead of magic numbers */
    napi_value argv[1];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != 1) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    /* Retrieve arguments */
    int32_t pcan_Channel;
    status = napi_get_value_int32(env, argv[0], &pcan_Channel);
    assert(status == napi_ok);

    /* Get current CAN status */
    TPCANStatus pcan_can_status;
    
    pcan_can_status = CAN_GetStatus(pcan_Channel);
    printf("pcan_CAN_GetStatus: 0x%02X (%s)\n", pcan_can_status, pcan_status_lookup(pcan_can_status));

    /* Create a N-API value for the status and return it */
    napi_value can_status;
    status = napi_create_bigint_uint64(env, pcan_can_status, &can_status);
    assert(status == napi_ok);

    return can_status;
}




napi_value pcan_CAN_Read(napi_env env, napi_callback_info info)
{
    napi_status status;

    /* Get callback info and arguments from N-API:
       TPCANHandle Channel (int32)
       TPCANMsg *MessageBuffer (uint64)
       TPCANTimestapm *TimestampBuffer */
    size_t argc = 3;
    napi_value argv[3];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != 3) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    /* Retrieve arguments */
    /* argv[0] - channel */
    int32_t pcan_Channel;
    status = napi_get_value_int32(env, argv[0], &pcan_Channel);
    assert(status == napi_ok);

    /* argv[1] - message buffer */
    napi_valuetype pcan_MessageBuffer_type;
    status = napi_typeof(env, argv[1], &pcan_MessageBuffer_type);
    assert(status == napi_ok);

    if (pcan_MessageBuffer_type != napi_object) {
	napi_throw_type_error(env, 0, "Argument 1 is not an object.");
	return 0;
    }

    /* argv[2] - timestamp buffer */
    napi_valuetype pcan_TimestampBuffer_type;
    status = napi_typeof(env, argv[2], &pcan_TimestampBuffer_type);
    assert(status == napi_ok);

    if (pcan_TimestampBuffer_type != napi_object) {
	napi_throw_type_error(env, 0, "Argument 2 is not an object.");
	return 0;
    }

    /* Read from CAN bus */
    TPCANMsg msg = { 0 };
    TPCANTimestamp timestamp = { 0 };
    TPCANStatus pcan_result;

    pcan_result = CAN_Read(pcan_Channel, &msg, &timestamp);
    printf("pcan_CAN_Read: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));

    pcan_dump_msg(&msg);
    pcan_dump_timestamp(&timestamp);

    /* Put read values into objects passed from N-API */
    /* arvg[1] - message buffer */
    napi_value pcan_MessageBuffer_value_id; /* uint32 */
    napi_value pcan_MessageBuffer_value_msgtype; /* uint32 */
    napi_value pcan_MessageBuffer_value_len; /* uint32 */
    napi_value pcan_MessageBuffer_value_data; /* buffer */
    void* pcan_MessageBuffer_value_data_result;
    
    status = napi_create_uint32(env, msg.ID, &pcan_MessageBuffer_value_id);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "id", pcan_MessageBuffer_value_id);
    assert(status == napi_ok);
    
    status = napi_create_uint32(env, msg.MSGTYPE, &pcan_MessageBuffer_value_msgtype);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "msgtype", pcan_MessageBuffer_value_msgtype);
    assert(status == napi_ok);
    
    status = napi_create_uint32(env, msg.LEN, &pcan_MessageBuffer_value_len);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "len", pcan_MessageBuffer_value_len);
    assert(status == napi_ok);

    status = napi_create_buffer_copy(env, msg.LEN, (void*)&(msg.DATA), (void*)&pcan_MessageBuffer_value_data_result, &pcan_MessageBuffer_value_data);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[1], "data", pcan_MessageBuffer_value_data);
    assert(status == napi_ok);

    /* argv[2] - timestamp buffer */
    napi_value pcan_TimestampBuffer_value_millis; /* uint32 */
    napi_value pcan_TimestampBuffer_value_millis_overflow; /* uint32 */
    napi_value pcan_TimestampBuffer_value_micros; /* uint32 */

    status = napi_create_uint32(env, timestamp.millis, &pcan_TimestampBuffer_value_millis);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[2], "millis", pcan_TimestampBuffer_value_millis);

    status = napi_create_uint32(env, timestamp.millis_overflow, &pcan_TimestampBuffer_value_millis_overflow);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[2], "millis_overflow", pcan_TimestampBuffer_value_millis_overflow);

    status = napi_create_uint32(env, timestamp.micros, &pcan_TimestampBuffer_value_micros);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[2], "micros", pcan_TimestampBuffer_value_micros);

    /* Create a N-API value for the result and return it */
    napi_value result;
    status = napi_create_int32(env, pcan_result, &result);

    return result;
}




napi_value pcan_CAN_Write(napi_env env, napi_callback_info info)
{
    napi_status status;

    /* Get calback info and arguments from N-API:
       TPCANHandle Channel (int32)
       TPCANMsg *MessageBuffer (uint64) */

    size_t argc = 2;
    napi_value argv[2];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != 2) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    /* Retrieve arguments */
    /* argv[0] - channel (TPCANHandle) */
    int32_t pcan_Channel;
    status = napi_get_value_int32(env, argv[0], &pcan_Channel);
    assert(status == napi_ok);

    /* argv[1] - message buffer (TPCANMsg) */
    napi_valuetype pcan_MessageBuffer_type;
    status = napi_typeof(env, argv[1], &pcan_MessageBuffer_type);
    assert(status == napi_ok);

    if (pcan_MessageBuffer_type != napi_object) {
	napi_throw_type_error(env, 0, "Argument 1 is not an object.");
	return 0;
    }

    /* Copy message contents from N-API object to TPCANMsg struct */
    TPCANMsg msg = { 0 };

    napi_value pcan_MessageBuffer_value_id;
    status = napi_get_named_property(env, argv[1], "id", &pcan_MessageBuffer_value_id);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcan_MessageBuffer_value_id, &(msg.ID));
    assert(status == napi_ok);

    napi_value pcan_MessageBuffer_value_msgtype;
    status = napi_get_named_property(env, argv[1], "msgtype", &pcan_MessageBuffer_value_msgtype);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcan_MessageBuffer_value_msgtype, (uint32_t*)&(msg.MSGTYPE));
    assert(status == napi_ok);

    napi_value pcan_MessageBuffer_value_len;
    status = napi_get_named_property(env, argv[1], "len", &pcan_MessageBuffer_value_len);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcan_MessageBuffer_value_len, (uint32_t*)&(msg.LEN));
    assert(status == napi_ok);

    napi_value pcan_MessageBuffer_value_data;
    BYTE *msg_tmp_DATA;
    size_t msg_tmp_LEN = 0;
    status = napi_get_named_property(env, argv[1], "data", &pcan_MessageBuffer_value_data);
    assert(status == napi_ok);
    status = napi_get_buffer_info(env, pcan_MessageBuffer_value_data, (void**)&msg_tmp_DATA, &msg_tmp_LEN);
    assert(status == napi_ok);

    if (msg_tmp_LEN != msg.LEN) {
	napi_throw_error(env, 0, "Mismatch between len argument and actual data buffer length");
	return 0;
    }

    memcpy(msg.DATA, msg_tmp_DATA, msg.LEN);

    /* Write to CAN bus */
    TPCANStatus pcan_result;

    pcan_result = CAN_Write(pcan_Channel, &msg);
    printf("pcan_CAN_Write: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));

    pcan_dump_msg(&msg);

    /* Create a N-API value for the result and return it */
    napi_value result;
    status = napi_create_int32(env, pcan_result, &result);
    
    return result;
}




napi_value pcan_CAN_GetValue(napi_env env, napi_callback_info info)
{
    napi_status status;

    /* Get callback info and arguments:
       TPCANHandle Channel (int32) */
    size_t argc = 3; /* TODO: make these constants somewhere, instead of magic numbers */
    napi_value argv[3];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != 3) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments. \n"
			      "The N-API implementation of CAN_GetValue only accepts Channel, Parameter, and Value.");
	return 0;
    }

    /* Retrieve arguments */
    /* argv[0] - channel */
    int32_t pcan_Channel;
    status = napi_get_value_int32(env, argv[0], &pcan_Channel);
    assert(status == napi_ok);

    /* argv[1] - parameter */
    int32_t pcan_Parameter;
    status = napi_get_value_int32(env, argv[1], &pcan_Parameter);
    assert(status == napi_ok);

    /* argv[2] - buffer */
    napi_valuetype pcan_Buffer_type;
    status = napi_typeof(env, argv[2], &pcan_Buffer_type);
    assert(status == napi_ok);

    if (pcan_Buffer_type != napi_object) {
	napi_throw_type_error(env, 0, "Argument 2 is not an object.");
	return 0;
    }

    /* argv[3] - buffer length (skipped) */
    uint32_t pcan_BufferLength;
    

    /* Some parameters return things other than a single uint32. These are currently
       outside of the scope of this project. */
    switch (pcan_Parameter) {
    case PCAN_LOG_LOCATION:
    case PCAN_LOG_TEXT:
    case PCAN_TRACE_LOCATION:
    case PCAN_IP_ADDRESS:
    case PCAN_ATTACHED_CHANNELS:
    case PCAN_HARDWARE_NAME:
	napi_throw_error(env, 0, "Requested parameter is currently unsupported.");
	return 0;
    default:
	break;
    }

    /* Create empty buffers */
    uint32_t pcan_Buffer = 0;
    pcan_BufferLength = sizeof(pcan_Buffer);
      
    /* Get requested value from CAN channel */
    TPCANStatus pcan_result;

    pcan_result = CAN_GetValue(pcan_Channel, pcan_Parameter, &pcan_Buffer, pcan_BufferLength);

    printf("pcan_CAN_GetValue: 0x%02X, 0x%02X (%s), 0x%02X, 0x%02X, 0x%02X (%s)\n",
	   pcan_Channel, pcan_Parameter, pcan_parameter_lookup(pcan_Parameter),
	   pcan_Buffer, pcan_BufferLength, pcan_result, pcan_status_lookup(pcan_result));
    /* TODO: replace instances of "pcan_result" with "pcan_status" */

    /* Put value into an the buffer object passed from N-API */
    napi_value pcan_Buffer_value;
    status = napi_create_uint32(env, pcan_Buffer, &pcan_Buffer_value);
    assert(status == napi_ok);
    status = napi_set_named_property(env, argv[2], "value", pcan_Buffer_value);
    assert(status == napi_ok);

    /* Create a N-API value for the result and return it */
    napi_value result;
    status = napi_create_int32(env, pcan_result, &result);

    return result;
}



enum { descriptors_len = 7 };

napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;

    /* Turn off buffering on stdout */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Build array of property descriptors */
    napi_property_descriptor descriptors[descriptors_len] = {
	DECLARE_NAPI_METHOD("CAN_Initialize", pcan_CAN_Initialize),
	DECLARE_NAPI_METHOD("CAN_Uninitialize", pcan_CAN_Uninitialize),
	DECLARE_NAPI_METHOD("CAN_Reset", pcan_CAN_Reset),
	DECLARE_NAPI_METHOD("CAN_GetStatus", pcan_CAN_GetStatus),
	DECLARE_NAPI_METHOD("CAN_Read", pcan_CAN_Read),
	DECLARE_NAPI_METHOD("CAN_Write", pcan_CAN_Write),
	DECLARE_NAPI_METHOD("CAN_GetValue", pcan_CAN_GetValue),
    };

    status = napi_define_properties(env, exports, descriptors_len, descriptors);
    assert(status == napi_ok);

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);
