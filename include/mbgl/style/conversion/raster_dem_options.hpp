#pragma once

#include <mbgl/style/sources/raster_dem_source.hpp>
#include <mbgl/style/conversion.hpp>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<RasterDEMOptions> {
    std::optional<RasterDEMOptions> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
