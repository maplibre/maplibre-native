#include <mbgl/webgpu/drawable_builder.hpp>
#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/webgpu/drawable_impl.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

DrawableBuilder::DrawableBuilder(std::string name_)
    : gfx::DrawableBuilder(std::move(name_)) {}

DrawableBuilder::~DrawableBuilder() = default;

std::unique_ptr<gfx::Drawable> DrawableBuilder::createDrawable() const {
    return std::make_unique<Drawable>(drawableName);
}

std::unique_ptr<gfx::Drawable::DrawSegment> DrawableBuilder::createSegment(gfx::DrawMode drawMode,
                                                                           SegmentBase&& segment) {
    return std::make_unique<Drawable::DrawSegment>(drawMode, std::move(segment));
}

void DrawableBuilder::init() {
    if (!currentDrawable) {
        mbgl::Log::Warning(mbgl::Event::Render, "DrawableBuilder::init called with no current drawable");
        return;
    }

    auto& drawable = static_cast<Drawable&>(*currentDrawable);

    // Set the vertex attribute ID so the drawable knows which shared vertex buffer to use
    drawable.setVertexAttrId(vertexAttrId);

    // Handle special case where rawVerticesCount is set but rawVertices is empty (from fills)
    // In this case, the vertex data is in the vertex attributes
    if (impl->rawVerticesCount && impl->rawVertices.empty()) {
        // The vertex data is in the vertex attributes, use updateVertexAttributes
        // Get the vertex attributes from the drawable (already set by base class flush())
        if (drawable.getVertexAttributes() && !impl->segments.empty()) {
            // Get the draw mode from the first segment
            gfx::DrawMode drawMode = impl->segments[0]->getMode();
            // The segments in the drawable will be set by setIndexData below
            // We just need to trigger the vertex attribute extraction
            // Pass empty segments for now since setIndexData will set them properly
            drawable.updateVertexAttributes(
                drawable.getVertexAttributes(), impl->rawVerticesCount, drawMode, impl->sharedIndexes, nullptr, 0);

        } else {
        }
    } else if (impl->rawVerticesCount && !impl->rawVertices.empty()) {
        auto raw = std::move(impl->rawVertices);
        drawable.setVertices(std::move(raw), impl->rawVerticesCount, impl->rawVerticesType);
    } else if (!impl->vertices.empty()) {
        const auto& verts = impl->vertices.vector();

        if (!verts.empty()) {
            constexpr auto vertSize = sizeof(std::remove_reference<decltype(verts)>::type::value_type);
            std::vector<uint8_t> raw(verts.size() * vertSize);
            std::memcpy(raw.data(), verts.data(), raw.size());
            drawable.setVertices(std::move(raw), verts.size(), gfx::AttributeDataType::Short2);
        }
    } else {
    }

    // Set index data if available
    if (!impl->sharedIndexes && !impl->buildIndexes.empty()) {
        impl->sharedIndexes = std::make_shared<gfx::IndexVectorBase>(std::move(impl->buildIndexes));
    }

    if (impl->sharedIndexes && impl->sharedIndexes->elements()) {
        drawable.setIndexData(impl->sharedIndexes, std::move(impl->segments));
    } else {
        mbgl::Log::Warning(mbgl::Event::Render, "  No index data to set!");
    }

    // Flush texture bindings so the shared builder doesn't leak the previous tile's
    // Texture2D handles into the next draw call. Other backends reset textures while
    // binding state; WebGPU relies on the builder being fresh per pass.
    for (auto& textureSlot : textures) {
        textureSlot.reset();
    }
}

} // namespace webgpu
} // namespace mbgl
