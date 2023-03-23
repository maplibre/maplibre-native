#pragma once

#include <mbgl/style/transition_options.hpp>
#include <mbgl/style/conversion.hpp>

#include <optional>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<TransitionOptions> {
public:
    std::optional<TransitionOptions> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
