#pragma once

#include <mbgl/util/traits.hpp>

#include <algorithm>
#include <cassert>
#include <string>
#include <optional>

namespace mbgl {

template <typename T>
class Enum {
public:
    using Type = T;
    static const char* toString(T);
    static std::optional<T> toEnum(const std::string&);
};

#define MBGL_DEFINE_ENUM(T, ...)                                                                       \
                                                                                                       \
    static const constexpr std::pair<const T, const char*> T##_names[] = __VA_ARGS__;                  \
                                                                                                       \
    template <>                                                                                        \
    const char* Enum<T>::toString(T t) {                                                               \
        auto it = std::find_if(                                                                        \
            std::begin(T##_names), std::end(T##_names), [&](const auto& v) { return t == v.first; });  \
        assert(it != std::end(T##_names));                                                             \
        return it->second;                                                                             \
    }                                                                                                  \
                                                                                                       \
    template <>                                                                                        \
    std::optional<T> Enum<T>::toEnum(const std::string& s) {                                           \
        auto it = std::find_if(                                                                        \
            std::begin(T##_names), std::end(T##_names), [&](const auto& v) { return s == v.second; }); \
        return it == std::end(T##_names) ? std::optional<T>() : it->first;                             \
    }

/// Generic union for `operator|` on bitmask enums
template <typename T> // `is_scoped_enum` is c++23
inline constexpr static std::enable_if_t<std::is_enum_v<T>, T> bitmaskEnumUnion(T x, T y) noexcept {
    return T{static_cast<std::underlying_type_t<T>>(mbgl::underlying_type(x) | mbgl::underlying_type(y))};
}
/// Generic union for `operator&` on bitmask enums
template <typename T>
inline constexpr static std::enable_if_t<std::is_enum_v<T>, T> bitmaskEnumIntersection(T x, T y) noexcept {
    return T{static_cast<std::underlying_type_t<T>>(mbgl::underlying_type(x) & mbgl::underlying_type(y))};
}

} // namespace mbgl
