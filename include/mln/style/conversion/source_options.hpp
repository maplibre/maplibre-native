#pragma once

#include <mln/style/sources/raster_dem_source.hpp>
#include <mln/style/conversion.hpp>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<SourceOptions> {
    std::optional<SourceOptions> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
