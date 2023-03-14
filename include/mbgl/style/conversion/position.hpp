#pragma once

#include <mbgl/style/position.hpp>
#include <mbgl/style/conversion.hpp>

#include <optional>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<Position> {
    std::optional<Position> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
