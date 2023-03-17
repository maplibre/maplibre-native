#pragma once

#include <mbgl/util/tileset.hpp>
#include <mbgl/style/conversion.hpp>

#include <optional>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<Tileset> {
public:
    std::optional<Tileset> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
