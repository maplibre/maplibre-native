//

//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cbridge/include/BaseHandle.h"
#include "cbridge/include/Export.h"

_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_ArrowPolylineConfig_create_handle(double headLength, double headAngle, _baseRef lineColor, double lineWidth);
_GLUECODIUM_C_EXPORT void GluecodiumPlugin_ArrowPolylineConfig_release_handle(_baseRef handle);

_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_ArrowPolylineConfig_create_optional_handle(double headLength, double headAngle, _baseRef lineColor, double lineWidth);
_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_ArrowPolylineConfig_unwrap_optional_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT void GluecodiumPlugin_ArrowPolylineConfig_release_optional_handle(_baseRef handle);

_GLUECODIUM_C_EXPORT double GluecodiumPlugin_ArrowPolylineConfig_headLength_get(_baseRef handle);
_GLUECODIUM_C_EXPORT double GluecodiumPlugin_ArrowPolylineConfig_headAngle_get(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef GluecodiumPlugin_ArrowPolylineConfig_lineColor_get(_baseRef handle);
_GLUECODIUM_C_EXPORT double GluecodiumPlugin_ArrowPolylineConfig_lineWidth_get(_baseRef handle);





#ifdef __cplusplus
}
#endif
