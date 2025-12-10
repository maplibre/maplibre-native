//

//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cbridge/include/BaseHandle.h"
#include "cbridge/include/Export.h"



_GLUECODIUM_C_EXPORT void GluecodiumPlugin_ArrowPolylineExample_release_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_ArrowPolylineExample_copy_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT const void* GluecodiumPlugin_ArrowPolylineExample_get_swift_object_from_wrapper_cache(_baseRef handle);
_GLUECODIUM_C_EXPORT void GluecodiumPlugin_ArrowPolylineExample_cache_swift_object_wrapper(_baseRef handle, const void* swift_pointer);
_GLUECODIUM_C_EXPORT void GluecodiumPlugin_ArrowPolylineExample_remove_swift_object_from_wrapper_cache(_baseRef handle);
_GLUECODIUM_C_EXPORT void* GluecodiumPlugin_ArrowPolylineExample_get_typed(_baseRef handle);



_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_ArrowPolylineExample_create();

_GLUECODIUM_C_EXPORT void GluecodiumPlugin_ArrowPolylineExample_addArrowPolyline(_baseRef _instance, _baseRef coordinates, _baseRef config);

_GLUECODIUM_C_EXPORT void GluecodiumPlugin_ArrowPolylineExample_removeArrowPolyline(_baseRef _instance);




#ifdef __cplusplus
}
#endif
