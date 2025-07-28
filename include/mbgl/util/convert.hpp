#pragma once

#include <mbgl/util/util.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <charconv>
#include <optional>
#include <string>

namespace mbgl {
namespace util {

template <typename To, typename From, std::size_t Size>
constexpr std::array<To, Size> convert(const std::array<From, Size>& from)
    requires(std::is_convertible_v<From, To>)
{
    std::array<To, Size> to{{}};
    std::copy(std::begin(from), std::end(from), std::begin(to));
    return to;
}

template <typename To, typename From, std::size_t Size>
constexpr std::array<To, Size> cast(const std::array<From, Size>& from)
    requires(std::is_assignable_v<To&, From>)
{
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

template <typename T = int>
std::enable_if_t<std::is_integral_v<T>, std::optional<T>>
str_to_int(const std::string& str, int base = 10) noexcept {
    T result;
    auto first = str.data();
    auto last = str.data() + str.size();
    auto [ptr, ec] = std::from_chars(first, last, result, base);
    if (ec == std::errc{}) {
        return result;
    }
    return std::nullopt;
}

} // namespace util
} // namespace mbgl
