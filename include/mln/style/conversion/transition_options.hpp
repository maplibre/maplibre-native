#pragma once

#include <mln/style/transition_options.hpp>
#include <mln/style/conversion.hpp>

#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<TransitionOptions> {
public:
    std::optional<TransitionOptions> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
