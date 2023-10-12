#pragma once

#include <type_traits>
#include <tuple>

namespace mbgl {

template <class... Ts>
struct TypeList {
    template <template <class...> class T, class... Ps>
    using ExpandInto = T<Ps..., Ts...>;
};

namespace detail {

template <class, class>
struct TypeCons;

template <class T, class... Ts>
struct TypeCons<T, TypeList<Ts...>> {
    using Type = TypeList<T, Ts...>;
};

template <class, template <class> class>
struct TypeFilter;

template <template <class> class Predicate>
struct TypeFilter<TypeList<>, Predicate> {
    using Type = TypeList<>;
};

template <template <class> class Predicate, class T, class... Ts>
struct TypeFilter<TypeList<T, Ts...>, Predicate> {
    using Tail = typename TypeFilter<TypeList<Ts...>, Predicate>::Type;
    using Type = std::conditional_t<Predicate<T>::value, typename TypeCons<T, Tail>::Type, Tail>;
};

template <class...>
struct TypeListConcat;

template <>
struct TypeListConcat<> {
    using Type = TypeList<>;
};

template <class... As>
struct TypeListConcat<TypeList<As...>> {
    using Type = TypeList<As...>;
};

template <class... As, class... Bs, class... Rs>
struct TypeListConcat<TypeList<As...>, TypeList<Bs...>, Rs...> {
    using Type = typename TypeListConcat<TypeList<As..., Bs...>, Rs...>::Type;
};

template <class...>
struct TypeListIndex;

// Base case, the type list starts with the specified type, the index is zero
template <class X, class... As>
struct TypeListIndex<X, TypeList<X, As...>> {
    static constexpr std::size_t value = 0;
};

// The type list starts with some other type, the index is one more than the index with that prefix removed
template <class X, class A, class... As>
struct TypeListIndex<X, TypeList<A, As...>> {
    static constexpr std::size_t value = 1 + TypeListIndex<X, TypeList<As...>>::value;
};

} // namespace detail

template <class TypeList, template <class> class Predicate>
using FilteredTypeList = typename detail::TypeFilter<TypeList, Predicate>::Type;

template <class... Ts>
using TypeListConcat = typename detail::TypeListConcat<Ts...>::Type;

/// The zero-based index of the type within a type list
template <class T, class... Ts>
static constexpr std::size_t TypeListIndex = detail::TypeListIndex<T, Ts...>::value;

} // namespace mbgl
