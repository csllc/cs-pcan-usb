#define DECLARE_NAPI_METHOD(name, func) \
      { name, 0, func, 0, 0, 0, napi_default, 0 }


/*void napi_value_dump(napi_env env, napi_value value);*/
void napi_value_dump(napi_env env, napi_value value, char* name);
char *napi_status_lookup(napi_status status);
char *napi_valuetype_lookup(napi_valuetype valuetype);
