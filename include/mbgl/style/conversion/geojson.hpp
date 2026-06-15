#pragma once

#include <mbgl/util/geojson.hpp>
#include <mbgl/style/conversion.hpp>

#include <optional>
#include <string>

namespace mbgl {
namespace style {
namespace conversion {

// Workaround until https://github.com/mapbox/mapbox-gl-native/issues/5623 is done.
std::optional<GeoJSON> parseGeoJSON(const std::string&, Error&);

// Serialize a GeoJSON value back to a JSON string. This is the public
// counterpart to parseGeoJSON: consumers that link the core through the
// amalgam (which exports only mbgl::* symbols) cannot reach the bundled
// mapbox::geojson::stringify directly.
std::string stringifyGeoJSON(const GeoJSON&);

template <>
struct Converter<GeoJSON> {
public:
    std::optional<GeoJSON> operator()(const Convertible&, Error&) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
