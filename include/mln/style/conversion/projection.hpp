#pragma once

#include <mln/style/conversion.hpp>
#include <mln/style/projection.hpp>

#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<Projection> {
public:
    std::optional<Projection> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
