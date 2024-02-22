#pragma once

#include <mbgl/math/minmax.hpp>

#include <type_traits>

namespace mbgl {
namespace util {

template <typename T>
auto clamp(std::enable_if_t<std::is_scalar_v<T>,T> value, T min_, T max_) noexcept {
    return max(min_, min(max_, value));
}

} // namespace util
} // namespace mbgl
