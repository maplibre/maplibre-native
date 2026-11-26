#pragma once

#include <mln/style/conversion.hpp>
#include <mln/style/rotation.hpp>

#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<style::Rotation> {
    std::optional<style::Rotation> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
