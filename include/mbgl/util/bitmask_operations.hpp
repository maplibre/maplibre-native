#pragma once

#include <mbgl/util/traits.hpp>
#include <mbgl/util/util.hpp>

namespace mbgl {

template <typename Enum>
constexpr Enum operator|(Enum a, Enum b) {
    static_assert(std::is_enum<Enum>::value, "Enum must be an enum type");
    return Enum(mbgl::underlying_type(a) | mbgl::underlying_type(b));
}

template <typename Enum>
constexpr Enum& operator|=(Enum& a, Enum b) {
    static_assert(std::is_enum<Enum>::value, "Enum must be an enum type");
    return (a = a | b);
}

template <typename Enum>
constexpr bool operator&(Enum a, Enum b) {
    static_assert(std::is_enum<Enum>::value, "Enum must be an enum type");
    return bool(mbgl::underlying_type(a) & mbgl::underlying_type(b));
}

template <typename Enum>
constexpr Enum operator~(Enum value) {
    static_assert(std::is_enum<Enum>::value, "Enum must be an enum type");
    return Enum(~mbgl::underlying_type(value));
}

} // namespace mbgl