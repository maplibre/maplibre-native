#pragma once

#include <mln/util/image.hpp>

namespace mln {
namespace util {

PremultipliedImage premultiply(UnassociatedImage&&);
UnassociatedImage unpremultiply(PremultipliedImage&&);

} // namespace util
} // namespace mln
