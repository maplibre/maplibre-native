//

//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cbridge/include/BaseHandle.h"
#include "cbridge/include/Export.h"
#include <stdint.h>

_GLUECODIUM_C_EXPORT void GluecodiumPlugin_MaplibrePlugin_release_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_MaplibrePlugin_copy_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT const void* GluecodiumPlugin_MaplibrePlugin_get_swift_object_from_wrapper_cache(_baseRef handle);
_GLUECODIUM_C_EXPORT void GluecodiumPlugin_MaplibrePlugin_cache_swift_object_wrapper(_baseRef handle,
                                                                                     const void* swift_pointer);
_GLUECODIUM_C_EXPORT void GluecodiumPlugin_MaplibrePlugin_remove_swift_object_from_wrapper_cache(_baseRef handle);
_GLUECODIUM_C_EXPORT void* GluecodiumPlugin_MaplibrePlugin_get_typed(_baseRef handle);

_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_MaplibrePlugin_create();

_GLUECODIUM_C_EXPORT uint64_t GluecodiumPlugin_MaplibrePlugin_ptr_get(_baseRef _instance);

#ifdef __cplusplus
}
#endif
