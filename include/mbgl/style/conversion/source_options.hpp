#pragma once

#include <mbgl/style/sources/raster_dem_source.hpp>
#include <mbgl/style/conversion.hpp>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<SourceOptions> {
    std::optional<SourceOptions> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
