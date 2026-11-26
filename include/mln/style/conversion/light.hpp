#pragma once

#include <mln/style/light.hpp>
#include <mln/style/conversion.hpp>

#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<Light> {
public:
    std::optional<Light> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
