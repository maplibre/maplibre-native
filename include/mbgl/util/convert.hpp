#pragma once

#include <mbgl/util/util.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <string_view>
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

/// Convert string to integral types
template <typename T>
std::enable_if_t<std::is_integral_v<T>, std::optional<T>> parse(std::string_view str, int base = 10) noexcept {
    static_assert(std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, int64_t> ||
                      std::is_same_v<T, uint64_t>,
                  "Unsupported integral type in mbgl::util::parse<T>.");
    char* end = nullptr;
    errno = 0;

    if constexpr (std::is_same_v<T, int32_t>) {
        long val = std::strtol(str.data(), &end, base); // NOLINT(bugprone-suspicious-stringview-data-usage)
        if (errno == 0 && end == str.data() + str.size()) return static_cast<T>(val);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        unsigned long val = std::strtoul(str.data(), &end, base); // NOLINT(bugprone-suspicious-stringview-data-usage)
        if (errno == 0 && end == str.data() + str.size()) return static_cast<T>(val);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        long long val = std::strtoll(str.data(), &end, base); // NOLINT(bugprone-suspicious-stringview-data-usage)
        if (errno == 0 && end == str.data() + str.size()) return static_cast<T>(val);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        unsigned long long val = std::strtoull(
            str.data(), &end, base); // NOLINT(bugprone-suspicious-stringview-data-usage)
        if (errno == 0 && end == str.data() + str.size()) return static_cast<T>(val);
    }

    return std::nullopt;
}

/// Convert string to floating point types
template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::optional<T>> parse(std::string_view str) noexcept {
    static_assert(std::is_same_v<T, float> || std::is_same_v<T, double> || std::is_same_v<T, long double>,
                  "Unsupported floating point type in mbgl::util::parse<T>.");
    char* end = nullptr;
    errno = 0;

    if constexpr (std::is_same_v<T, float>) {
        float val = std::strtof(str.data(), &end); // NOLINT(bugprone-suspicious-stringview-data-usage)
        if (errno == 0 && end == str.data() + str.size()) return val;
    } else if constexpr (std::is_same_v<T, double>) {
        double val = std::strtod(str.data(), &end); // NOLINT(bugprone-suspicious-stringview-data-usage)
        if (errno == 0 && end == str.data() + str.size()) return val;
    } else if constexpr (std::is_same_v<T, long double>) {
        long double val = std::strtold(str.data(), &end); // NOLINT(bugprone-suspicious-stringview-data-usage)
        if (errno == 0 && end == str.data() + str.size()) return val;
    }

    return std::nullopt;
}

} // namespace util
} // namespace mbgl
