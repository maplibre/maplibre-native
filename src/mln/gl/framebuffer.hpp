#pragma once

#include <mln/gl/object.hpp>
#include <mln/util/size.hpp>

namespace mln {
namespace gl {

class Framebuffer {
public:
    Size size;
    gl::UniqueFramebuffer framebuffer;
};

} // namespace gl
} // namespace mln
