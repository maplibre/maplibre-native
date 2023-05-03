#include <mbgl/gl/drawable_gl_builder.hpp>

#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {
namespace gl {

gfx::DrawablePtr DrawableGLBuilder::createDrawable() const {
    return std::make_shared<DrawableGL>(drawableName.empty() ? name : drawableName);
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
    if (auto colorAttr = attrs.getOrAdd("a_color")) {
        // We should have either a single color or one per vertex.  Otherwise,
        // the color mode was probably changed after vertexes were added.
        if (impl->colors.size() > 1 && impl->colors.size() != impl->vertices.elements()) {
            impl->colors.clear();
        }

        if (impl->colors.empty()) {
            colorAttr->set(0, DrawableGL::colorAttrValue(getColor()));
        } else {
            std::size_t index = 0;
            colorAttr->reserve(impl->colors.size());
            for (const auto& color : impl->colors) {
                colorAttr->set(index++, DrawableGL::colorAttrValue(color));
            }
        }
    }

    constexpr auto indexOffset = 0;
    const auto indexCount = impl->indexes.elements();
    drawableGL.setIndexData(impl->indexes.vector(), indexOffset, indexCount);

    impl->indexes.clear();
    impl->vertices.clear();
    impl->colors.clear();
}

} // namespace gl
} // namespace mbgl
