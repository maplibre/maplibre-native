#pragma once

#include <mln/util/ignore.hpp>

#include <cstdint>
#include <functional>
#include <type_traits>

namespace mln {
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

/// Mixes the bits of `value` (the finaliser of splitmix64), so that sums of mixed values rarely collide.
constexpr std::uint64_t mix(std::uint64_t value) noexcept {
    value += 0x9e3779b97f4a7c15;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9;
    value = (value ^ (value >> 27)) * 0x94d049bb133111eb;
    return value ^ (value >> 31);
}
} // namespace detail

/// Generate a hash key from a collection of integer values which doesn't depend on their order.
/// Each value is mixed before it is added: with plain sums and products of the values, sets of ten small values
/// already collided.
template <typename TIter, typename TKey = detail::TIterVal<TIter>>
TKey order_independent_hash(std::remove_const_t<TIter> cur, const TIter end) noexcept {
    std::uint64_t sum = 0;
    for (; cur != end; ++cur) {
        sum += detail::mix(static_cast<std::uint64_t>(*cur));
    }
    return static_cast<TKey>(sum);
}

} // namespace util
} // namespace mln
