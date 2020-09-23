#ifndef _HELPER_NAPI_H_
#define _HELPER_NAPI_H_

#include <node_api.h>

/* N-API helpers */
#define DECLARE_NAPI_METHOD(name, func) \
      { name, 0, func, 0, 0, 0, napi_default, 0 }

void napi_value_dump(napi_env env, napi_value value, char* name);
char *napi_status_lookup(napi_status status);
char *napi_valuetype_lookup(napi_valuetype valuetype);
bool napi_check(napi_env env, napi_status status);
void napi_dump_error_info(napi_extended_error_info *error_info);



#endif /* _HELPER_NAPI_H_ */
