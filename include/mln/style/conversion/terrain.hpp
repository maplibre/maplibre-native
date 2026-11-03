#pragma once

#include <mln/style/terrain.hpp>
#include <mln/style/conversion.hpp>

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
