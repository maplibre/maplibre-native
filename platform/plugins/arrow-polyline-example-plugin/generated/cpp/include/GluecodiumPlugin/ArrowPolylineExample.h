// -------------------------------------------------------------------------------------------------
//

//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "GluecodiumPlugin/ArrowPolylineConfig.h"
#include "GluecodiumPlugin/LatLng.h"
#include "GluecodiumPlugin/MaplibrePlugin.h"
#include "glue_internal/ExportGluecodiumCpp.h"
#include "glue_internal/VectorHash.h"
#include <memory>
#include <vector>

namespace GluecodiumPlugin {
/**
 * An example plugin that draws arrow polylines on the map.
 * Takes a list of coordinates and draws a polyline with a chevron arrow head.

 */
class _GLUECODIUM_CPP_EXPORT ArrowPolylineExample: public ::GluecodiumPlugin::MaplibrePlugin {
public:
    ArrowPolylineExample();
    virtual ~ArrowPolylineExample();


public:
    /**
     *
     * \return @NotNull
     */
    static ::std::shared_ptr< ::GluecodiumPlugin::ArrowPolylineExample > create(  );
    /**
     * Add an arrow polyline to the map
     * \param[in] coordinates List of LatLng coordinates (at least 2 points)
     * \param[in] config Arrow appearance configuration
     */
    virtual void add_arrow_polyline( const ::std::vector< ::GluecodiumPlugin::LatLng >& coordinates, const ::GluecodiumPlugin::ArrowPolylineConfig& config ) = 0;
    /**
     * Remove the current arrow polyline from the map
     */
    virtual void remove_arrow_polyline(  ) = 0;
};


}
