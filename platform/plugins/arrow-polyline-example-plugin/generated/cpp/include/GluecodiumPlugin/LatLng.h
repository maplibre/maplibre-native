// -------------------------------------------------------------------------------------------------
//

//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "glue_internal/ExportGluecodiumCpp.h"
#include "glue_internal/Hash.h"

namespace GluecodiumPlugin {
/**
 * A geographic coordinate with latitude and longitude

 */
struct _GLUECODIUM_CPP_EXPORT LatLng {
    double latitude;
    double longitude;

    LatLng();
    LatLng(double latitude, double longitude);

    bool operator==(const LatLng& rhs) const;
    bool operator!=(const LatLng& rhs) const;
};

} // namespace GluecodiumPlugin

namespace glue_internal {
template <>
struct hash<::GluecodiumPlugin::LatLng> {
    _GLUECODIUM_CPP_EXPORT std::size_t operator()(const ::GluecodiumPlugin::LatLng& t) const;
};
} // namespace glue_internal
