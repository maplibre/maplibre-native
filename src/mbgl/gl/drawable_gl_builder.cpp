#include <mbgl/gl/drawable_gl_builder.hpp>
#include <mbgl/gl/drawable_gl.hpp>

namespace mbgl {
namespace gl {

gfx::DrawablePtr DrawableGLBuilder::createDrawable() const {
    return std::make_shared<DrawableGL>();
};

} // namespace gl
} // namespace mbgl

