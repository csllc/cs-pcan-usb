#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include <windows.h>

#include "helper.h"

napi_value CAN_initialize(napi_env env, napi_callback_info info)
{
    napi_status status;

    /* Anticipate 2 arguments:
       TPCANHandle Channel (int32)
       TPCANBaudrate Btr0Btr1 (int32)
    */
    size_t argc = 2;
    napi_value argv[2];

    /* Get callback info and argments */
    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != 2) {
	napi_throw_type_error(env, 0, "Incorrect number of arguments.");
	return 0;
    }

    napi_value_dump(env, argv[0], "argv[0]");

    napi_value ret;
    napi_create_int32(env, 0, &ret);

    napi_value_dump(env, ret, "ret");

    return ret;

}

napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;

    printf("pcan.cc: Init\n");

    napi_property_descriptor desc_CAN_initialize =
	DECLARE_NAPI_METHOD("CAN_initialize", CAN_initialize);

    status = napi_define_properties(env, exports, 1, &desc_CAN_initialize);
    assert(status == napi_ok);

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);
