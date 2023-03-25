#pragma once

#include <mbgl/style/light.hpp>
#include <mbgl/style/conversion.hpp>

#include <optional>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<Light> {
public:
    std::optional<Light> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
