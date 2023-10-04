#include <mbgl/gl/drawable_gl_builder.hpp>

#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/gl/drawable_gl_impl.hpp>
#include <mbgl/util/convert.hpp>

#include <cstring>

namespace mbgl {
namespace gl {

gfx::UniqueDrawable DrawableGLBuilder::createDrawable() const {
    return std::make_unique<DrawableGL>(drawableName.empty() ? name : drawableName);
};

std::unique_ptr<gfx::Drawable::DrawSegment> DrawableGLBuilder::createSegment(gfx::DrawMode drawMode,
                                                                             SegmentBase&& seg) {
    return std::make_unique<DrawableGL::DrawSegmentGL>(drawMode, std::move(seg), VertexArray{{nullptr, false}});
}

void DrawableGLBuilder::init() {
    auto& drawableGL = static_cast<DrawableGL&>(*currentDrawable);

    drawableGL.setVertexAttrNameId(vertexAttrNameId);

    if (impl->rawVerticesCount) {
        auto raw = impl->rawVertices;
        drawableGL.setVertices(std::move(raw), impl->rawVerticesCount, impl->rawVerticesType);
    } else {
        const auto& verts = impl->vertices.vector();
        constexpr auto vertSize = sizeof(std::remove_reference<decltype(verts)>::type::value_type);
        std::vector<uint8_t> raw(verts.size() * vertSize);
        std::memcpy(raw.data(), verts.data(), raw.size());
        drawableGL.setVertices(std::move(raw), verts.size(), gfx::AttributeDataType::Short2);
    }

    if (!impl->sharedIndexes && !impl->buildIndexes.empty()) {
        impl->sharedIndexes = std::make_shared<gfx::IndexVectorBase>(std::move(impl->buildIndexes));
    }
    drawableGL.setIndexData(std::move(impl->sharedIndexes), std::move(impl->segments));

    impl->buildIndexes.clear();
    impl->segments.clear();
    impl->vertices.clear();
    textures.clear();
}

} // namespace gl
} // namespace mbgl
