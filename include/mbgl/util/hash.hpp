#pragma once

#include <mbgl/util/ignore.hpp>

#include <functional>
#include <type_traits>

namespace mbgl {
namespace util {

template <class T>
void hash_combine(std::size_t& seed, const T& v) noexcept {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <class... Args>
std::size_t hash(Args&&... args) noexcept {
    std::size_t seed = 0;
    ignore({(hash_combine(seed, args), 0)...});
    return seed;
}

namespace detail {
template <typename TIter>
using TIterVal = std::enable_if_t<std::is_integral_v<typename TIter::value_type>, typename TIter::value_type>;
} // namespace detail

template <typename T>
constexpr T factor() noexcept {
    // Default factor prime value from 64-bit FNV hash.
    if constexpr (sizeof(T) == 8) return 1099511628211;
    // for 32-bit hash
    return 16777619;
}

/// Generate a hash key from a collection of integer values which doesn't depend on their order.
/// Adapted from https://stackoverflow.com/a/76993810/135138
template <typename TIter, typename TKey = detail::TIterVal<TIter>, TKey factor = factor<TKey>()>
TKey order_independent_hash(std::remove_const_t<TIter> cur, const TIter end) noexcept {
    detail::TIterVal<TIter> sum = 0, product = 1;
    for (; cur != end; ++cur) {
        const auto& value = *cur;
        sum += value;
        product *= (1 + value) * factor;
    }
    return sum ^ product;
}

} // namespace util
} // namespace mbgl
