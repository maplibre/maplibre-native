//

//

#include "cbridge/include/GenericCollections.h"
#include "GluecodiumPlugin/LatLng.h"
#include "cbridge_internal/include/BaseHandleImpl.h"
#include "glue_internal/VectorHash.h"
#include <optional>
#include <vector>
#include <new>

_baseRef ArrayOf_GluecodiumPlugin_LatLng_create_handle() {
    return reinterpret_cast<_baseRef>( new ::std::vector< ::GluecodiumPlugin::LatLng >( ) );
}

_baseRef ArrayOf_GluecodiumPlugin_LatLng_copy_handle(_baseRef handle) {
    return reinterpret_cast<_baseRef>( new ::std::vector< ::GluecodiumPlugin::LatLng >( *reinterpret_cast<::std::vector< ::GluecodiumPlugin::LatLng >*>( handle ) ) );
}

void ArrayOf_GluecodiumPlugin_LatLng_release_handle(_baseRef handle) {
    delete reinterpret_cast<::std::vector< ::GluecodiumPlugin::LatLng >*>( handle );
}

uint64_t ArrayOf_GluecodiumPlugin_LatLng_count(_baseRef handle) {
    return Conversion<::std::vector< ::GluecodiumPlugin::LatLng >>::toCpp( handle ).size( );
}

_baseRef ArrayOf_GluecodiumPlugin_LatLng_get( _baseRef handle, uint64_t index ) {
    return Conversion<::GluecodiumPlugin::LatLng>::referenceBaseRef(Conversion<::std::vector< ::GluecodiumPlugin::LatLng >>::toCpp( handle )[index]);
}

void ArrayOf_GluecodiumPlugin_LatLng_append( _baseRef handle, _baseRef item )
{
    Conversion<::std::vector< ::GluecodiumPlugin::LatLng >>::toCpp(handle).push_back(Conversion<::GluecodiumPlugin::LatLng>::toCpp(item));
}

_baseRef ArrayOf_GluecodiumPlugin_LatLng_create_optional_handle() {
    return reinterpret_cast<_baseRef>( new ( ::std::nothrow ) std::optional<::std::vector< ::GluecodiumPlugin::LatLng >>( ::std::vector< ::GluecodiumPlugin::LatLng >( ) ) );
}

void ArrayOf_GluecodiumPlugin_LatLng_release_optional_handle(_baseRef handle) {
    delete reinterpret_cast<std::optional<::std::vector< ::GluecodiumPlugin::LatLng >>*>( handle );
}

_baseRef ArrayOf_GluecodiumPlugin_LatLng_unwrap_optional_handle(_baseRef handle) {
    return reinterpret_cast<_baseRef>( &**reinterpret_cast<std::optional<::std::vector< ::GluecodiumPlugin::LatLng >>*>( handle ) );
}


