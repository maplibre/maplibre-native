#pragma once

#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/conversion.hpp>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<GeoJSONOptions> {
    std::optional<GeoJSONOptions> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
