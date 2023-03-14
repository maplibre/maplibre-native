#pragma once

#include <mbgl/style/conversion.hpp>
#include <mbgl/style/rotation.hpp>

#include <optional>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<style::Rotation> {
    std::optional<style::Rotation> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
