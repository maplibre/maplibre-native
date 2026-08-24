#pragma once

#include <mln/style/sprite.hpp>
#include <mln/style/conversion.hpp>

#include <optional>
#include <memory>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<Sprite> {
public:
    std::optional<Sprite> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
