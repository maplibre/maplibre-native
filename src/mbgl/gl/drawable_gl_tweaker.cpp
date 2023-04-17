#include <mbgl/gl/drawable_gl_tweaker.hpp>
#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/renderer/paint_parameters.hpp>

namespace mbgl {
namespace gl {

void DrawableGLTweaker::execute(gfx::Drawable& drawable, const PaintParameters &) {
    auto& drawGL = (gl::DrawableGL&)drawable;
    // apply expressions based on current state, update attributes, uniforms, etc.
    drawGL.getId();
}

} // namespace gl
} // namespace mbgl

