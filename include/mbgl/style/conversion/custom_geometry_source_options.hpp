#pragma once

#include <mbgl/style/conversion.hpp>
#include <mbgl/style/sources/custom_geometry_source.hpp>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<CustomGeometrySource::Options> {
    std::optional<CustomGeometrySource::Options> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
