#pragma once

#include <mln/style/layer.hpp>
#include <mln/style/conversion.hpp>

#include <memory>
#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<std::unique_ptr<Layer>> {
public:
    std::optional<std::unique_ptr<Layer>> operator()(const Convertible& value, Error& error) const;
};

std::optional<Error> setPaintProperties(Layer& layer, const Convertible& value);

} // namespace conversion
} // namespace style
} // namespace mln
