//

//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cbridge/include/BaseHandle.h"
#include "cbridge/include/Export.h"
#include <stdint.h>

_GLUECODIUM_C_EXPORT _baseRef ArrayOf_GluecodiumPlugin_LatLng_create_handle();
_GLUECODIUM_C_EXPORT _baseRef ArrayOf_GluecodiumPlugin_LatLng_copy_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT void ArrayOf_GluecodiumPlugin_LatLng_release_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT uint64_t ArrayOf_GluecodiumPlugin_LatLng_count(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef ArrayOf_GluecodiumPlugin_LatLng_get(_baseRef handle, uint64_t index);
_GLUECODIUM_C_EXPORT void ArrayOf_GluecodiumPlugin_LatLng_append(_baseRef handle, _baseRef item);

_GLUECODIUM_C_EXPORT _baseRef ArrayOf_GluecodiumPlugin_LatLng_create_optional_handle();
_GLUECODIUM_C_EXPORT void ArrayOf_GluecodiumPlugin_LatLng_release_optional_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef ArrayOf_GluecodiumPlugin_LatLng_unwrap_optional_handle(_baseRef handle);

#ifdef __cplusplus
}
#endif
