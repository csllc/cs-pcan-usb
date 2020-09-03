#include <stdio.h>
#include <node_api.h>
#include <stdint.h>
#include <string.h>

#include "helper.h"

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
