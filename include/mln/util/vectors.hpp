#pragma once

#include <mln/util/math.hpp>

#include <array>
#include <algorithm>
#include <utility>
#include <type_traits>

namespace mln {

using vec2 = std::array<double, 2>;
using vec2f = std::array<float, 2>;
using vec3 = std::array<double, 3>;
using vec3f = std::array<float, 3>;
using vec3i = std::array<int, 3>;
using vec4 = std::array<double, 4>;

template <typename Type, std::size_t... sizes>
auto concatenate(const std::array<Type, sizes>&... arrays) {
    std::array<Type, (sizes + ...)> result;
    std::size_t index{};

    ((std::copy_n(arrays.begin(), sizes, result.begin() + index), index += sizes), ...);

    return result;
}

// Extract a portion of a std::array.
// vec4::slice<0,2> is equivalent to OpenGL vec4.xy
template <std::size_t Start, std::size_t Count, typename Type, std::size_t N>
auto slice(const std::array<Type, N>& array) {
    static_assert(Start + Count <= N, "Slice out of bounds");
    return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return std::array<Type, Count>{{array[Start + I]...}};
    }(std::make_index_sequence<Count>{});
}

namespace vector {

// Allow vector arithmetic operations

inline vec2 operator+(const vec2& a, const vec2& b) {
    return vec2{a[0] + b[0], a[1] + b[1]};
}
inline vec2 operator-(const vec2& a, double b) {
    return vec2{a[0] - b, a[1] - b};
}
inline vec4 operator+(const vec4& a, const vec4& b) {
    return vec4{a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]};
}

inline vec2 operator*(const vec2& a, const vec2& b) {
    return vec2{a[0] * b[0], a[1] * b[1]};
}
inline vec2 operator*(const double a, const vec2& b) {
    return vec2{a * b[0], a * b[1]};
}
inline vec2 operator*(const vec2& a, const double b) {
    return vec2{a[0] * b, a[1] * b};
}

inline vec2 operator/(const vec2& a, const vec2& b) {
    return vec2{a[0] / b[0], a[1] / b[1]};
}
inline vec2 operator/(const vec2& a, const double b) {
    return vec2{a[0] / b, a[1] / b};
}
inline vec3 operator/(const vec3& a, const double b) {
    return vec3{a[0] / b, a[1] / b, a[2] / b};
}

inline vec2 gl_fmod(const vec2& a, const double b) {
    return {util::gl_fmod(a[0], b), util::gl_fmod(a[1], b)};
}

namespace detail {
template <typename T>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
inline constexpr bool is_std_array_v = is_std_array<T>::value;

template <typename T, typename U, size_t N, size_t... Is>
auto convert_array(const std::array<U, N>& arr, std::index_sequence<Is...>) {
    return std::array<T, N>{{static_cast<T>(arr[Is])...}};
}

template <typename T, typename U>
auto as_array(U&& value) {
    using Decayed = std::remove_cv_t<std::remove_reference_t<U>>;
    if constexpr (is_std_array_v<Decayed>) {
        return convert_array<T>(value, std::make_index_sequence<std::tuple_size_v<Decayed>>{});
    } else {
        return std::array<T, 1>{{static_cast<T>(std::forward<U>(value))}};
    }
}
} // namespace detail

// Allow OpenGL style vector construction, e.g. vec<double>(1.0, 2.0, 3.0, 4.0) or vec<double>(vec2{1.0, 2.0}, 3.0, 4.0)
template <typename T = double, typename... Args>
auto vec(Args&&... args) {
    return concatenate(detail::as_array<T>(std::forward<Args>(args))...);
}

inline vec2 max(const vec2& a, double b) {
    return vec2{std::max(a[0], b), std::max(a[1], b)};
}
inline vec2 max(const vec2& a, const vec2& b) {
    return vec2{std::max(a[0], b[0]), std::max(a[1], b[1])};
}

} // namespace vector
} // namespace mln
