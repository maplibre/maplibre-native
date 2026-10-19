#pragma once

#include <nonstd/expected.hpp>

namespace mln {

template <class T, class E>
using expected = nonstd::expected<T, E>;

template <class E>
using unexpected = nonstd::unexpected_type<E>;

} // namespace mln
