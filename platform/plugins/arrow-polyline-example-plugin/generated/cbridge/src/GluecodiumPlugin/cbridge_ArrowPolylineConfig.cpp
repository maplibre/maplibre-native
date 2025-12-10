//

//

#include "cbridge/include/GluecodiumPlugin/cbridge_ArrowPolylineConfig.h"
#include "GluecodiumPlugin/ArrowPolylineConfig.h"
#include "cbridge/include/StringHandle.h"
#include "cbridge_internal/include/BaseHandleImpl.h"
#include <memory>
#include <new>
#include <optional>
#include <string>

_baseRef
GluecodiumPlugin_ArrowPolylineConfig_create_handle( double headLength, double headAngle, _baseRef lineColor, double lineWidth )
{
    ::GluecodiumPlugin::ArrowPolylineConfig* _struct = new ( ::std::nothrow ) ::GluecodiumPlugin::ArrowPolylineConfig();
    _struct->head_length = headLength;
    _struct->head_angle = headAngle;
    _struct->line_color = Conversion<::std::string>::toCpp( lineColor );
    _struct->line_width = lineWidth;

    return reinterpret_cast<_baseRef>( _struct );
}

void
GluecodiumPlugin_ArrowPolylineConfig_release_handle( _baseRef handle )
{
    delete get_pointer<::GluecodiumPlugin::ArrowPolylineConfig>( handle );
}

_baseRef
GluecodiumPlugin_ArrowPolylineConfig_create_optional_handle(double headLength, double headAngle, _baseRef lineColor, double lineWidth)
{
    auto _struct = new ( ::std::nothrow ) std::optional<::GluecodiumPlugin::ArrowPolylineConfig>( ::GluecodiumPlugin::ArrowPolylineConfig( ) );
    (*_struct)->head_length = headLength;
    (*_struct)->head_angle = headAngle;
    (*_struct)->line_color = Conversion<::std::string>::toCpp( lineColor );
    (*_struct)->line_width = lineWidth;

    return reinterpret_cast<_baseRef>( _struct );
}

_baseRef
GluecodiumPlugin_ArrowPolylineConfig_unwrap_optional_handle( _baseRef handle )
{
    return reinterpret_cast<_baseRef>( &**reinterpret_cast<std::optional<::GluecodiumPlugin::ArrowPolylineConfig>*>( handle ) );
}

void GluecodiumPlugin_ArrowPolylineConfig_release_optional_handle(_baseRef handle) {
    delete reinterpret_cast<std::optional<::GluecodiumPlugin::ArrowPolylineConfig>*>( handle );
}

double GluecodiumPlugin_ArrowPolylineConfig_headLength_get(_baseRef handle) {
    auto struct_pointer = get_pointer<const ::GluecodiumPlugin::ArrowPolylineConfig>(handle);
    return struct_pointer->head_length;
}
double GluecodiumPlugin_ArrowPolylineConfig_headAngle_get(_baseRef handle) {
    auto struct_pointer = get_pointer<const ::GluecodiumPlugin::ArrowPolylineConfig>(handle);
    return struct_pointer->head_angle;
}
_baseRef GluecodiumPlugin_ArrowPolylineConfig_lineColor_get(_baseRef handle) {
    auto struct_pointer = get_pointer<const ::GluecodiumPlugin::ArrowPolylineConfig>(handle);
    return Conversion<::std::string>::toBaseRef(struct_pointer->line_color);
}
double GluecodiumPlugin_ArrowPolylineConfig_lineWidth_get(_baseRef handle) {
    auto struct_pointer = get_pointer<const ::GluecodiumPlugin::ArrowPolylineConfig>(handle);
    return struct_pointer->line_width;
}




