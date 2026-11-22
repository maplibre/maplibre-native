#pragma once

// TEMPORARY WORKAROUND. The OpenHarmony SDK's clang rejects the alias declaration
// `using underlying_type_t = underlying_type<T>::type;` at
// vendor/maplibre-tile-spec/cpp/include/mlt/common.hpp:25 because it lacks
// `typename` before the dependent type (C++20 implicit typename is not supported
// by that compiler). This copy of the vendored header adds the `typename` and is
// injected ahead of the vendored include directory by platform/ohos/ohos.cmake.
//
// The real fix is to upstream the `typename` to maplibre/maplibre-tile-spec and
// bump the vendored submodule, after which this file and the include-directory
// override in ohos.cmake should be deleted. Keep this file otherwise identical to
// the vendored header.

#include <mlt/polyfill.hpp>

#include <string_view>
#include <type_traits>

namespace mlt {

using DataView = std::string_view;

template <typename T, std::size_t N>
constexpr std::size_t countof(T (&)[N]) {
    return N;
}

/// `std::underlying_type` that doesn't fail when given a simple type
template <typename T, bool = std::is_enum_v<T>>
struct underlying_type {
    using type = T;
};
template <typename T>
struct underlying_type<T, true> : ::std::underlying_type<T> {};
template <class T>
using underlying_type_t = typename underlying_type<T>::type;

} // namespace mlt
