#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <mapbox/variant.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace mbgl {

template <typename... T>
using variant = mapbox::util::variant<T...>;

// https://en.cppreference.com/w/cpp/utility/variant/visit
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace mbgl
