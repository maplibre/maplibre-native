#pragma once

#include <mbgl/style/source.hpp>
#include <mbgl/style/conversion.hpp>

#include <memory>
#include <optional>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<std::unique_ptr<Source>> {
public:
    std::optional<std::unique_ptr<Source>> operator()(const Convertible& value,
                                                      Error& error,
                                                      const std::string& id) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
