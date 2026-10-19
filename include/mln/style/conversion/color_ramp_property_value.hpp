#pragma once

#include <mln/style/color_ramp_property_value.hpp>
#include <mln/style/conversion.hpp>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<ColorRampPropertyValue> {
    std::optional<ColorRampPropertyValue> operator()(const Convertible& value,
                                                     Error& error,
                                                     bool /* allowDataExpressions */ = false,
                                                     bool /* convertTokens */ = false) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
