#pragma once

#include <mln/style/conversion.hpp>
#include <mln/style/sky.hpp>

#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<Sky> {
public:
    std::optional<Sky> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
