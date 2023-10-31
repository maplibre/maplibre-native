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
using TIterVal = std::enable_if_t<std::is_integral<typename TIter::value_type>::value, typename TIter::value_type>;
} // namespace detail

/// Generate a hash key from a collection of integer values which doesn't depend on their order.
/// Adapted from https://stackoverflow.com/a/76993810/135138
/// Default factor prime value from 64-bit FNV hash.
template <typename TIter, typename TKey = detail::TIterVal<TIter>, TKey factor = 1099511628211LL>
TKey order_independent_hash(TIter cur, const TIter end) {
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
