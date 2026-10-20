#pragma once

#include <mln/util/traits.hpp>
#include <mln/util/util.hpp>

namespace mln {

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum operator|(Enum a, Enum b) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    return Enum(mln::underlying_type(a) | mln::underlying_type(b));
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum& operator|=(Enum& a, Enum b) {
    return (a = a | b);
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr bool operator&(Enum a, Enum b) {
    return bool(mln::underlying_type(a) & mln::underlying_type(b));
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum operator~(Enum value) {
    return Enum(~mln::underlying_type(value));
}

} // namespace mln
