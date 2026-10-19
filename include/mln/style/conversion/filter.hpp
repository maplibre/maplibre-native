#pragma once

#include <mln/style/filter.hpp>
#include <mln/style/conversion.hpp>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<Filter> {
public:
    std::optional<Filter> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
