#pragma once

#include <mln/style/sources/geojson_source.hpp>
#include <mln/style/conversion.hpp>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<GeoJSONOptions> {
    std::optional<GeoJSONOptions> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
