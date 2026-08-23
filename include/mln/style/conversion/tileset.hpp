#pragma once

#include <mln/util/tileset.hpp>
#include <mln/style/conversion.hpp>

#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<Tileset> {
public:
    std::optional<Tileset> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
