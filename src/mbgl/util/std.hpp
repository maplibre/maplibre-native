#pragma once

#include <type_traits>
#include <utility>

namespace mbgl {
namespace util {

template <typename Container, typename ForwardIterator, typename Predicate>
void erase_if(Container& container, ForwardIterator it, Predicate pred) {
    while (it != container.end()) {
        if (pred(*it)) {
            it = container.erase(it);
        } else {
            ++it;
        }
    }
}

template <typename Container, typename Predicate>
void erase_if(Container& container, Predicate pred) {
    erase_if(container, container.begin(), pred);
}

} // namespace util
} // namespace mbgl

namespace std {
#if !__has_cpp_attribute(__cpp_lib_to_underlying)
template <typename E>
constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}
#endif
} // namespace std
