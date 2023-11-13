#pragma once

#include <mbgl/style/sprite.hpp>
#include <mbgl/style/conversion.hpp>

#include <optional>
#include <memory>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<std::unique_ptr<Sprite>> {
public:
    std::optional<std::unique_ptr<Sprite>> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
