//

//

#include "cbridge/include/GluecodiumPlugin/cbridge_LatLng.h"
#include "GluecodiumPlugin/LatLng.h"
#include "cbridge_internal/include/BaseHandleImpl.h"
#include <memory>
#include <new>
#include <optional>

_baseRef
GluecodiumPlugin_LatLng_create_handle( double latitude, double longitude )
{
    ::GluecodiumPlugin::LatLng* _struct = new ( ::std::nothrow ) ::GluecodiumPlugin::LatLng();
    _struct->latitude = latitude;
    _struct->longitude = longitude;

    return reinterpret_cast<_baseRef>( _struct );
}

void
GluecodiumPlugin_LatLng_release_handle( _baseRef handle )
{
    delete get_pointer<::GluecodiumPlugin::LatLng>( handle );
}

_baseRef
GluecodiumPlugin_LatLng_create_optional_handle(double latitude, double longitude)
{
    auto _struct = new ( ::std::nothrow ) std::optional<::GluecodiumPlugin::LatLng>( ::GluecodiumPlugin::LatLng( ) );
    (*_struct)->latitude = latitude;
    (*_struct)->longitude = longitude;

    return reinterpret_cast<_baseRef>( _struct );
}

_baseRef
GluecodiumPlugin_LatLng_unwrap_optional_handle( _baseRef handle )
{
    return reinterpret_cast<_baseRef>( &**reinterpret_cast<std::optional<::GluecodiumPlugin::LatLng>*>( handle ) );
}

void GluecodiumPlugin_LatLng_release_optional_handle(_baseRef handle) {
    delete reinterpret_cast<std::optional<::GluecodiumPlugin::LatLng>*>( handle );
}

double GluecodiumPlugin_LatLng_latitude_get(_baseRef handle) {
    auto struct_pointer = get_pointer<const ::GluecodiumPlugin::LatLng>(handle);
    return struct_pointer->latitude;
}
double GluecodiumPlugin_LatLng_longitude_get(_baseRef handle) {
    auto struct_pointer = get_pointer<const ::GluecodiumPlugin::LatLng>(handle);
    return struct_pointer->longitude;
}




