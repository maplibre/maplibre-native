#pragma once

#include <mapbox/geojson.hpp>
#include <mln/util/feature.hpp>

namespace mln {

using GeoJSON = mapbox::geojson::geojson;
using FeatureCollection = mapbox::geojson::feature_collection;
using FeatureExtensionValue = mapbox::util::variant<Value, FeatureCollection>;

} // namespace mln
