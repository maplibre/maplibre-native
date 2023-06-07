#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace mbgl {
namespace util {

template <typename Container, typename ForwardIterator, typename Predicate>
void erase_if(Container &container, ForwardIterator it, Predicate pred) {
    while (it != container.end()) {
        if (pred(*it)) {
            it = container.erase(it);
        } else {
            ++it;
        }
    }
}

template <typename Container, typename Predicate>
void erase_if(Container &container, Predicate pred) {
    erase_if(container, container.begin(), pred);
}

template <typename T, std::size_t N, std::size_t M>
auto concat(const std::array<T, N>& a1, const std::array<T, M>& a2) {
    std::array<T, N + M> result;
    std::copy(a1.cbegin(), a1.cend(), result.begin());
    std::copy(a2.cbegin(), a2.cend(), result.begin() + N);
    return result;
}

} // namespace util
} // namespace mbgl
