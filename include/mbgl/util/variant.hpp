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

} // namespace mbgl
