//

//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cbridge/include/BaseHandle.h"
#include "cbridge/include/Export.h"
#include <stdbool.h>

_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_LatLng_create_handle(double latitude, double longitude);
_GLUECODIUM_C_EXPORT void GluecodiumPlugin_LatLng_release_handle(_baseRef handle);

_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_LatLng_create_optional_handle(double latitude, double longitude);
_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_LatLng_unwrap_optional_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT void GluecodiumPlugin_LatLng_release_optional_handle(_baseRef handle);

_GLUECODIUM_C_EXPORT double GluecodiumPlugin_LatLng_latitude_get(_baseRef handle);
_GLUECODIUM_C_EXPORT double GluecodiumPlugin_LatLng_longitude_get(_baseRef handle);





#ifdef __cplusplus
}
#endif
