#pragma once

#include <mln/util/traits.hpp>
#include <mln/util/util.hpp>

namespace mln {

template <typename Enum>
constexpr Enum operator|(Enum a, Enum b) {
    static_assert(std::is_enum_v<Enum>, "Enum must be an enum type");
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    return Enum(mln::underlying_type(a) | mln::underlying_type(b));
}

template <typename Enum>
constexpr Enum& operator|=(Enum& a, Enum b) {
    static_assert(std::is_enum_v<Enum>, "Enum must be an enum type");
    return (a = a | b);
}

template <typename Enum>
constexpr bool operator&(Enum a, Enum b) {
    static_assert(std::is_enum_v<Enum>, "Enum must be an enum type");
    return bool(mln::underlying_type(a) & mln::underlying_type(b));
}

template <typename Enum>
constexpr Enum operator~(Enum value) {
    static_assert(std::is_enum_v<Enum>, "Enum must be an enum type");
    return Enum(~mln::underlying_type(value));
}

} // namespace mln
