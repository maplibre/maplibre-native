#pragma once

#include <mln/util/geojson.hpp>

namespace mln {

class CRSWorldCoordinateHelper;
class WorldCoordinateHelper;

/// The GeoJSON with every position moved from lat/lng to the CRS's world square, x and y from 0 to 1 with y growing
/// down, which geojson-vt and supercluster tile with unit square coordinates. Lines and rings that reach past the CRS's
/// latitude range move onto it, and an edge that then runs along it gets a position every 10 degrees of longitude, so
/// what lies past it, like the opposite pole of a polar projection, ends up along it, off the map. Positions without a
/// finite place in the world square are left out. The positions go through the converter in batches of up to
/// `maxConverterBatch`.
GeoJSON reprojectGeoJSONToWorld(const GeoJSON&, const CRSWorldCoordinateHelper&);

/// The features with every position moved from the world square back to lat/lng, in batches of up to
/// `maxConverterBatch`.
mapbox::feature::feature_collection<double> reprojectFeaturesFromWorld(mapbox::feature::feature_collection<double>,
                                                                       const WorldCoordinateHelper&);

} // namespace mln
