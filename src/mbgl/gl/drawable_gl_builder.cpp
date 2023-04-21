#include <mbgl/gl/drawable_gl_builder.hpp>

#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/gl/drawable_gl.hpp>

namespace mbgl {
namespace gl {

gfx::DrawablePtr DrawableGLBuilder::createDrawable() const {
    return std::make_shared<DrawableGL>();
};

void DrawableGLBuilder::init() {
    auto& drawableGL = static_cast<DrawableGL&>(*currentDrawable);

    auto& attrs = drawableGL.mutableVertexAttributes();
    if (auto posAttr = attrs.getOrAdd("a_pos")) {
        std::size_t index = 0;
        for (const auto& vert : impl->vertices.vector()) {
            posAttr->set(index++, gfx::VertexAttribute::int2{vert.a1[0], vert.a1[1]});
        }
    }

    constexpr auto indexOffset = 0;
    const auto indexCount = impl->indexes.elements();
    drawableGL.setIndexData(impl->indexes.vector(), indexOffset, indexCount);
}

} // namespace gl
} // namespace mbgl

