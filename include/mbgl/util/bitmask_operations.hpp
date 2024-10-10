#pragma once

#include <mbgl/util/traits.hpp>
#include <mbgl/util/util.hpp>

namespace mbgl {

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum operator|(Enum a, Enum b) {
    return Enum(mbgl::underlying_type(a) | mbgl::underlying_type(b));
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum& operator|=(Enum& a, Enum b) {
    return (a = a | b);
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr bool operator&(Enum a, Enum b) {
    return bool(mbgl::underlying_type(a) & mbgl::underlying_type(b));
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum operator~(Enum value) {
    return Enum(~mbgl::underlying_type(value));
}

} // namespace mbgl