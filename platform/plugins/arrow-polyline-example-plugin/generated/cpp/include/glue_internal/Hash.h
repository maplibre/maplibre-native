// -------------------------------------------------------------------------------------------------
//

//
// -------------------------------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>

namespace glue_internal {
/**
 * Hash-like function object for `std::shared_ptr`, comparing the contained class instead
 * of the pointer value. Specializations are generated for classes implementing `operator==`.
 */
template <class T>
struct EqualityHash {
    EqualityHash() = delete;
};

/**
 * Equality-like function object to be used in conjunction with `SharedPointerHash` above
 * when using equatable classes as keys for `std::unordered_map` or `std::unordered_set`.
 */
template <class T>
struct EqualityEqualTo {
    bool operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const {
        if (!lhs && !rhs) {
            return true;
        }
        if (!lhs || !rhs) {
            return false;
        }
        return *lhs == *rhs;
    }
};

template <class T>
struct EnumHash {
    std::size_t operator()(const T& t) const { return static_cast<std::size_t>(t); }
};

template <class T, class SfinaePlaceholder = void>
struct SelectHash {
    using type = std::hash<T>;
};

template <class T>
struct SelectHash<T, typename std::enable_if<std::is_constructible<EqualityHash<T>>::value>::type> {
    using type = EqualityHash<T>;
};

template <class T>
struct SelectHash<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    using type = EnumHash<T>;
};

/*
 * Generic hash functional, it uses SharedPointerHash or EnumHash if possible and falls back to
 * std::hash otherwise. Types for which hash is defined specialize this template.
 */
template <class T>
struct hash {
    size_t operator()(const T& t) const noexcept { return typename SelectHash<T>::type()(t); }
};

template <class T>
struct hash<const T> {
    size_t operator()(const T& t) const noexcept { return hash<T>()(t); }
};

template <class T>
struct hash<std::optional<T>> {
    size_t operator()(const std::optional<T>& t) const noexcept { return t.has_value() ? hash<T>()(*t) : 0; }
};

} // namespace glue_internal
