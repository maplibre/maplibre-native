#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/gl/drawable_gl_impl.hpp>

namespace mbgl {
namespace gl {

DrawableGL::DrawableGL()
    : impl(std::make_unique<Impl>()) {
}

DrawableGL::~DrawableGL() {
}

void DrawableGL::draw(const PaintParameters &parameters) const
{
    impl->draw(parameters);
}

} // namespace gl
} // namespace mbgl
