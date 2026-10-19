#pragma once

#include <mln/style/conversion.hpp>
#include <mln/util/geo.hpp>

#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<LatLng> {
public:
    std::optional<LatLng> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
