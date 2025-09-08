#pragma once

#include <type_traits>
#include <algorithm>
#include <cmath>

namespace mbgl {
namespace util {

template <typename T>
T max(T a, T b)
    requires(std::is_integral_v<T>)
{
    return std::max(a, b);
}

template <typename T>
T max(T a, T b)
    requires(std::is_floating_point_v<T>)
{
    return ::fmax(a, b);
}

template <typename T, typename... Ts>
T max(T a, T b, Ts... args)
    requires(std::is_arithmetic_v<T>)
{
    return max(a, max(b, args...));
}

template <typename T>
T min(T a, T b)
    requires(std::is_integral_v<T>)
{
    return std::min(a, b);
}

template <typename T>
T min(T a, T b)
    requires(std::is_floating_point_v<T>)
{
    return ::fmin(a, b);
}

template <typename T, typename... Ts>
T min(T a, T b, Ts... args)
    requires(std::is_arithmetic_v<T>)
{
    return min(a, min(b, args...));
}

} // namespace util
} // namespace mbgl
