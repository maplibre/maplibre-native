#pragma once

#include <mbgl/style/terrain.hpp>
#include <mbgl/style/conversion.hpp>

#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<Terrain> {
public:
    std::optional<Terrain> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
