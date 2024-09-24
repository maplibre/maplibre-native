#pragma once

#include <concepts>
#include <iterator>
#include <memory>
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

namespace detail {
/// Output iterator that inserts elements into an already-ordered container in order
template <typename TTarget, typename TComp>
struct OrderedInserter {
    using iterator_category = std::output_iterator_tag;
    using difference_type = std::ptrdiff_t;

    std::reference_wrapper<TTarget> target;
    TComp compare;

    template <typename T>
    void operator=(T&& item) {
        const auto it = std::upper_bound(target.get().begin(), target.get().end(), item, compare);
        target.get().insert(it, std::forward<T>(item));
    }
    OrderedInserter& operator*() { return *this; }
    OrderedInserter& operator++() { return *this; }
    OrderedInserter& operator++(int) { return *this; }
    OrderedInserter& operator--() { return *this; }
    OrderedInserter& operator--(int) { return *this; }
};
} // namespace detail

/// Type inference for `OrderedInserter`, default comparator
template <typename T>
auto make_ordered_inserter(T& target) {
    using namespace detail;
    static_assert(std::movable<OrderedInserter<T, std::less<>>>);
    static_assert(std::weakly_incrementable<OrderedInserter<T, std::less<>>>);
    return OrderedInserter<T, std::less<>>{target, {}};
}

/// Type inference for `OrderedInserter`, custom  comparator
template <typename TTarget, typename TComp>
auto make_ordered_inserter(TTarget& target, TComp compare) {
    using namespace detail;
    static_assert(std::movable<OrderedInserter<TTarget, TComp>>);
    static_assert(std::weakly_incrementable<OrderedInserter<TTarget, TComp>>);
    return OrderedInserter<TTarget, TComp>{target, std::forward<TComp>(compare)};
}

} // namespace util
} // namespace mbgl
