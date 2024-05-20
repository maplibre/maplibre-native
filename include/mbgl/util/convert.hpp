#pragma once

#include <mbgl/util/util.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>

namespace mbgl {
namespace util {

template <typename To, typename From, std::size_t Size, typename = std::enable_if_t<std::is_convertible_v<From, To>>>
constexpr std::array<To, Size> convert(const std::array<From, Size>& from) {
    std::array<To, Size> to{{}};
    std::copy(std::begin(from), std::end(from), std::begin(to));
    return to;
}

template <typename To, typename From, std::size_t Size, typename = std::enable_if_t<std::is_assignable_v<To&, From>>>
constexpr std::array<To, Size> cast(const std::array<From, Size>& from) {
    std::array<To, Size> to{{}};
    std::transform(std::begin(from), std::end(from), std::begin(to), [](From x) { return static_cast<To>(x); });
    return to;
}

/// Convert an `N`-tuple of all `T`s to `array<T,N>`
template <typename... Ts>
inline constexpr auto to_array(std::tuple<Ts...>&& tuple) {
    return std::apply([](auto&&... x) { return std::array{std::forward<decltype(x)>(x)...}; },
                      std::forward<std::tuple<Ts...>>(tuple));
}

} // namespace util
} // namespace mbgl
