#include <stdio.h>
#include <node_api.h>
#include <stdint.h>
#include <string.h>

#include "helper.h"
#include "PCANBasic.h"

/* Iterate through a given napi_value's properties, and print the complete
   contents to stdout */
void napi_value_dump(napi_env env, napi_value value, char* name)
{
    napi_status status;

    /* Get value type */
    napi_valuetype vt;

    status = napi_typeof(env, value, &vt);
    if (status != napi_ok) {
	printf("napi_value_dump: Error at napi_typeof: %s",
	       napi_status_lookup(status));
	return;
    }

    /* Build value string in buffer */

    char napi_value_buffer[128] = {0};
    
    bool napi_value_bool = 0;
    int64_t napi_value_int64 = 0;
    char napi_value_string_buffer[128] = {0};
    size_t napi_value_string_result;
    void* napi_value_external_ptr = 0;

    switch (vt) {
    case napi_undefined:
	snprintf(napi_value_buffer, sizeof(napi_value_buffer), "undefined");
	break;
    case napi_null:
	snprintf(napi_value_buffer, sizeof(napi_value_buffer), "null");
	break;
    case napi_boolean:
	status = napi_get_value_bool(env, value, &napi_value_bool);
	if (status != napi_ok) {
	    printf("napi_value_dump: Error at napi_get_value_bool: %s",
		   napi_status_lookup(status));
	}
	snprintf(napi_value_buffer, sizeof(napi_value_buffer), "%s",
		 napi_value_bool ? "true" : "false");
	break;
    case napi_number: /* Assume int64 here, even though not always most appropriate */
	status = napi_get_value_int64(env, value, &napi_value_int64);
	if (status != napi_ok) {
	    printf("napi_value_dump: Error at napi_get_value_int64: %s",
		   napi_status_lookup(status));
	}
	snprintf(napi_value_buffer, sizeof(napi_value_buffer), "0x%llX = %lli", (uint64_t)napi_value_int64, napi_value_int64);
	break;
    case napi_string:
	status = napi_get_value_string_utf8(env,
					    value,
					    napi_value_string_buffer,
					    sizeof(napi_value_string_buffer),
					    &napi_value_string_result);
	if (status != napi_ok) {
	    printf("napi_value_dump: Error at napi_get_string_utf8: %s",
		   napi_status_lookup(status));
	}
	strncpy(napi_value_buffer, napi_value_string_buffer, sizeof(napi_value_buffer));
	break;
    case napi_symbol:
	snprintf(napi_value_buffer, sizeof(napi_value_buffer), "[symbol]");
	break;
    case napi_object:
	snprintf(napi_value_buffer, sizeof(napi_value_buffer), "[object]");
	break;
    case napi_function:
	snprintf(napi_value_buffer, sizeof(napi_value_buffer), "[function]");
	break;
    case napi_external:
	status = napi_get_value_external(env, value, &napi_value_external_ptr);
	if (status != napi_ok) {
	    printf("napi_value_dump: Error at napi_get_external: %s",
		   napi_status_lookup(status));
	}
	snprintf(napi_value_buffer, sizeof(napi_value_buffer),
		 "%p", napi_value_external_ptr);
	break;
    case napi_bigint: /* assume int64 here */
	status = napi_get_value_bigint_int64(env, value, &napi_value_int64, &napi_value_bool);
	if (status != napi_ok) {
	    printf("napi_value_dump: Error at napi_get_value_bigint_int64: %s",
		   napi_status_lookup(status));
	}
	snprintf(napi_value_buffer, sizeof(napi_value_buffer), "0x%llX = %lli (lossless=%s)",
		 (uint64_t)napi_value_int64,
		 napi_value_int64,
		 napi_value_bool ? "true" : "false");
	break;
    }

    /* Print result */
    if (name != 0) {
	printf("napi_value_dump: (%s) %s = %s\n",
	       napi_valuetype_lookup(vt),
	       name,
	       napi_value_buffer);
    } else {
	printf("napi_value_dump: (%s) = %s\n",
	       napi_valuetype_lookup(vt),
	       napi_value_buffer);
    }

    return;
}


struct {
    int code;
    char* string;
} status_codes[] = {
    { napi_ok, "napi_ok" },
    { napi_invalid_arg, "napi_invalid_arg" },
    { napi_object_expected, "napi_object_expected" },
    { napi_string_expected, "napi_string_expected" },
    { napi_name_expected, "napi_name_expected" },
    { napi_function_expected, "napi_function_expected" },
    { napi_number_expected, "napi_number_expected" },
    { napi_boolean_expected, "napi_boolean_expected" },
    { napi_array_expected, "napi_array_expected" },
    { napi_generic_failure, "napi_generic_failure" },
    { napi_pending_exception, "napi_pending_exception" },
    { napi_cancelled, "napi_cancelled" },
    { napi_escape_called_twice, "napi_escape_called_twice" },
    { napi_handle_scope_mismatch, "napi_handle_scope_mismatch" },
    { napi_callback_scope_mismatch, "napi_callback_scope_mismatch" },
    { napi_queue_full, "napi_queue_full" },
    { napi_closing, "napi_closing" },
    { napi_bigint_expected, "napi_bigint_expected" },
    { napi_date_expected, "napi_date_expected" },
    { napi_arraybuffer_expected, "napi_arraybuffer_expected" },
    { napi_detachable_arraybuffer_expected, "napi_detachable_arraybuffer_expected" },
    /*{ napi_would_deadlock, "napi_would_deadlock" },*/
    { 0, 0 }
};


char *napi_status_lookup(napi_status status)
{
    int i;
    for (i = 0; status_codes[i].string; i++) {
	if (status_codes[i].code == status) {
	    return status_codes[i].string;
	}
    }

    return "unknown";
}


struct {
    int valuetype;
    char* string;
} valuetypes[] = {
    { napi_undefined, "napi_undefined" },
    { napi_null, "napi_null" },
    { napi_boolean, "napi_boolean" },
    { napi_number, "napi_number" },
    { napi_string, "napi_string" },
    { napi_symbol, "napi_symbol" },
    { napi_object, "napi_object" },
    { napi_function, "napi_function" },
    { napi_external, "napi_external" },
    { napi_bigint, "napi_bigint" },
    { 0, 0}
};


char *napi_valuetype_lookup(napi_valuetype valuetype)
{
    int i;

    for (i = 0; valuetypes[i].string; i++) {
	if (valuetypes[i].valuetype == valuetype) {
	    return valuetypes[i].string;
	}
    }

    return "unknown";
}


bool napi_check(napi_env env, napi_status status)
{
    napi_extended_error_info *error_info = NULL;
    bool exception_pending = false;

    if (status != napi_ok) {
	napi_get_last_error_info(env, &error_info);
	napi_is_exception_pending(env, &exception_pending);

	if (!exception_pending) {
	    napi_dump_error_info(error_info);
	    napi_throw_error(env, NULL,
			     error_info->error_message);
	}

	return false;
    }

    return true;
}


void napi_dump_error_info(napi_extended_error_info *error_info)
{
    printf("error_info {\n"
	   "  error_message = \"%s\"\n"
	   "  engine_reserved = %p\n"
	   "  engine_error_code = 0x%X\n"
	   "  error_code = 0x%X\n"
	   "  }\n",
	   error_info->error_message,
	   error_info->engine_reserved,
	   error_info->engine_error_code,
	   error_info->error_code);

    return;
}


struct {
    int status;
    char* string;
} statuses[] = {
    { PCAN_ERROR_OK, "PCAN_ERROR_OK" },
    { PCAN_ERROR_XMTFULL, "PCAN_ERROR_XMTFULL" },
    { PCAN_ERROR_OVERRUN, "PCAN_ERROR_OVERRUN" },
    { PCAN_ERROR_BUSLIGHT, "PCAN_ERROR_BUSLIGHT" },
    { PCAN_ERROR_BUSHEAVY, "PCAN_ERROR_BUSHEAVY" },
    { PCAN_ERROR_BUSWARNING, "PCAN_ERROR_BUSWARNING" },
    { PCAN_ERROR_BUSPASSIVE, "PCAN_ERROR_BUSPASSIVE" },
    { PCAN_ERROR_BUSOFF, "PCAN_ERROR_BUSOFF" },
    { PCAN_ERROR_ANYBUSERR, "PCAN_ERROR_ANYBUSERR" },
    { PCAN_ERROR_QRCVEMPTY, "PCAN_ERROR_QRCVEMPTY" },
    { PCAN_ERROR_QOVERRUN, "PCAN_ERROR_QOVERRUN" },
    { PCAN_ERROR_QXMTFULL, "PCAN_ERROR_QXMTFULL" },
    { PCAN_ERROR_REGTEST, "PCAN_ERROR_REGTEST" },
    { PCAN_ERROR_NODRIVER, "PCAN_ERROR_NODRIVER" },
    { PCAN_ERROR_HWINUSE, "PCAN_ERROR_HWINUSE" },
    { PCAN_ERROR_NETINUSE, "PCAN_ERROR_NETINUSE" },
    { PCAN_ERROR_ILLHW, "PCAN_ERROR_ILLHW" },
    { PCAN_ERROR_ILLNET, "PCAN_ERROR_ILLNET" },
    { PCAN_ERROR_ILLCLIENT, "PCAN_ERROR_ILLCLIENT" },
    { PCAN_ERROR_ILLHANDLE, "PCAN_ERROR_ILLHANDLE" },
    { PCAN_ERROR_RESOURCE, "PCAN_ERROR_RESOURCE" },
    { PCAN_ERROR_ILLPARAMTYPE, "PCAN_ERROR_ILLPARAMTYPE" },
    { PCAN_ERROR_ILLPARAMVAL, "PCAN_ERROR_ILLPARAMVAL" },
    { PCAN_ERROR_UNKNOWN, "PCAN_ERROR_UNKNOWN" },
    { PCAN_ERROR_ILLDATA, "PCAN_ERROR_ILLDATA" },
    { PCAN_ERROR_ILLMODE, "PCAN_ERROR_ILLMODE" },
    { PCAN_ERROR_CAUTION, "PCAN_ERROR_CAUTION" },
    { PCAN_ERROR_INITIALIZE, "PCAN_ERROR_INITIALIZE" },
    { PCAN_ERROR_ILLOPERATION, "PCAN_ERROR_ILLOPERATION" },
};


char *pcan_status_lookup(TPCANStatus status)
{
    int i;

    for (i = 0; statuses[i].string; i++) {
	if (statuses[i].status == status) {
	    return statuses[i].string;
	}
    }

    return "status unknown";
}


void pcan_dump_msg(TPCANMsg *msg)
{
    int i;

    printf("msg { \n"
	     "  ID = 0x%X\n"
	     "  MSGTYPE = 0x%X (%s)\n"
	     "  LEN = 0x%X\n"
	     "  DATA = {",
	   msg->ID,
	   msg->MSGTYPE,
	   pcan_msgtype_lookup(msg->MSGTYPE),
	   msg->LEN);

    for (i=0; i < (msg->LEN); i++) {
	printf("0x%X", msg->DATA[i]);
	if (i != ((msg->LEN) - 1)) {
	    printf(", ");
	}
    }

    printf("}\n"
	   "}\n");

    return;
}



struct {
    int type;
    char* string;
} types [] = {
    { PCAN_MESSAGE_STANDARD, "PCAN_MESSAGE_STANDARD" },
    { PCAN_MESSAGE_RTR, "PCAN_MESSAGE_RTR" },
    { PCAN_MESSAGE_EXTENDED, "PCAN_MESSAGE_EXTENDED" },
    { PCAN_MESSAGE_FD, "PCAN_MESSAGE_FD" },
    { PCAN_MESSAGE_BRS, "PCAN_MESSAGE_BRS" },
    { PCAN_MESSAGE_ESI, "PCAN_MESSAGE_ESI" },
    { PCAN_MESSAGE_ERRFRAME, "PCAN_MESSAGE_ERRFRAME" },
    { PCAN_MESSAGE_STATUS, "PCAN_MESSAGE_STATUS" },
    { 0, 0 }
};

char *pcan_msgtype_lookup(TPCANMessageType type)
{
    int i;

    for (i = 0; types[i].string; i++) {
	if (types[i].type == type) {
	    return types[i].string;
	}
    }

    return "msgtype unknown";
}


void pcan_dump_timestamp(TPCANTimestamp *timestamp)
{
    printf("timestamp {\n"
	   "  millis = %i\n"
	   "  millis_overflow = %i\n"
	   "  micros = %i\n"
	   "}\n",
	   timestamp->millis,
	   timestamp->millis_overflow,
	   timestamp->micros);

    return;
}
