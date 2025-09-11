#include <mbgl/webgpu/drawable_builder.hpp>
#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/webgpu/drawable_impl.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

DrawableBuilder::DrawableBuilder(std::string name)
    : gfx::DrawableBuilder(std::move(name)) {}

DrawableBuilder::~DrawableBuilder() = default;

std::unique_ptr<gfx::Drawable> DrawableBuilder::createDrawable() const {
    Log::Info(Event::General, "WebGPU DrawableBuilder::createDrawable called for: " + name);
    return std::make_unique<Drawable>(name);
}

std::unique_ptr<gfx::Drawable::DrawSegment> DrawableBuilder::createSegment(gfx::DrawMode drawMode, SegmentBase&& segment) {
    return std::make_unique<Drawable::DrawSegment>(drawMode, std::move(segment));
}

void DrawableBuilder::init() {
    if (!currentDrawable) {
        return;
    }
    
    Log::Info(Event::General, "DrawableBuilder::init - mode: " + std::to_string(static_cast<int>(impl->getMode())) +
              ", polylineVertices: " + std::to_string(impl->polylineVertices.elements()));
    
    auto& drawable = static_cast<Drawable&>(*currentDrawable);
    
    // Handle special case where rawVerticesCount is set but rawVertices is empty (from fills)
    // In this case, the vertex data is in the vertex attributes
    if (impl->rawVerticesCount && impl->rawVertices.empty()) {
        Log::Info(Event::General, "DrawableBuilder: rawVerticesCount=" + std::to_string(impl->rawVerticesCount) +
                   " but rawVertices empty - vertex data is in attributes, calling updateVertexAttributes");
        // The vertex data is in the vertex attributes, use updateVertexAttributes
        // Get the vertex attributes from the drawable (already set by base class flush())
        if (drawable.getVertexAttributes() && !impl->segments.empty()) {
            // Get the draw mode from the first segment
            gfx::DrawMode drawMode = impl->segments[0]->getMode();
            // The segments in the drawable will be set by setIndexData below
            // We just need to trigger the vertex attribute extraction
            // Pass empty segments for now since setIndexData will set them properly
            drawable.updateVertexAttributes(drawable.getVertexAttributes(), 
                                           impl->rawVerticesCount,
                                           drawMode,
                                           impl->sharedIndexes,
                                           nullptr,
                                           0);
        }
    } else if (impl->rawVerticesCount && !impl->rawVertices.empty()) {
        Log::Info(Event::General, "DrawableBuilder: Setting raw vertices, count: " + std::to_string(impl->rawVerticesCount) +
                   ", size: " + std::to_string(impl->rawVertices.size()));
        auto raw = std::move(impl->rawVertices);
        drawable.setVertices(std::move(raw), impl->rawVerticesCount, impl->rawVerticesType);
    } else if (!impl->vertices.empty()) {
        const auto& verts = impl->vertices.vector();
        Log::Info(Event::General, "DrawableBuilder: vertices.empty=" + std::to_string(impl->vertices.empty()) +
                   ", vertices.elements=" + std::to_string(impl->vertices.elements()) +
                  ", vector.size=" + std::to_string(verts.size()));
        
        if (!verts.empty()) {
            constexpr auto vertSize = sizeof(std::remove_reference<decltype(verts)>::type::value_type);
            std::vector<uint8_t> raw(verts.size() * vertSize);
            std::memcpy(raw.data(), verts.data(), raw.size());
            drawable.setVertices(std::move(raw), verts.size(), gfx::AttributeDataType::Short2);
        }
    } else {
        Log::Info(Event::General, "DrawableBuilder: No vertices to set");
    }
    
    // Set index data if available
    if (!impl->sharedIndexes && !impl->buildIndexes.empty()) {
        impl->sharedIndexes = std::make_shared<gfx::IndexVectorBase>(std::move(impl->buildIndexes));
    }
    
    if (impl->sharedIndexes && impl->sharedIndexes->elements()) {
        drawable.setIndexData(impl->sharedIndexes, std::move(impl->segments));
    }
}

} // namespace webgpu
} // namespace mbgl