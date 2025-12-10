// -------------------------------------------------------------------------------------------------
//

//
// -------------------------------------------------------------------------------------------------

#include "GluecodiumPlugin/LatLng.h"
#include <utility>

namespace GluecodiumPlugin {

LatLng::LatLng( )
    : latitude{ }, longitude{ }
{
}

LatLng::LatLng( double latitude, double longitude )
    : latitude( std::move( latitude ) ), longitude( std::move( longitude ) )
{
}

bool LatLng::operator==( const LatLng& rhs ) const
{
    return latitude == rhs.latitude &&
        longitude == rhs.longitude;
}

bool LatLng::operator!=( const LatLng& rhs ) const
{
    return !( *this == rhs );
}


}

namespace glue_internal {
std::size_t
hash< ::GluecodiumPlugin::LatLng >::operator( )( const ::GluecodiumPlugin::LatLng& t ) const
{
    size_t hash_value = 43;
    hash_value = ( hash_value ^ ::glue_internal::hash< decltype( ::GluecodiumPlugin::LatLng::latitude ) >( )( t.latitude ) ) << 1;
    hash_value = ( hash_value ^ ::glue_internal::hash< decltype( ::GluecodiumPlugin::LatLng::longitude ) >( )( t.longitude ) ) << 1;
    return hash_value;
}
}
