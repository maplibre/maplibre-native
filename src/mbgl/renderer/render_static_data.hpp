#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/renderbuffer.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/renderer/buckets/heatmap_bucket.hpp>
#include <mbgl/renderer/buckets/raster_bucket.hpp>

#include <string>
#include <optional>

namespace mbgl {
namespace gfx {
class Context;
class UploadPass;
} // namespace gfx

class RenderStaticData {
public:
    RenderStaticData(std::unique_ptr<gfx::ShaderRegistry>&& shaders_);

    void upload(gfx::UploadPass&);

    std::optional<gfx::VertexBuffer<gfx::Vertex<PositionOnlyLayoutAttributes>>> tileVertexBuffer;
    std::optional<gfx::VertexBuffer<RasterLayoutVertex>> rasterVertexBuffer;
    std::optional<gfx::VertexBuffer<HeatmapTextureLayoutVertex>> heatmapTextureVertexBuffer;

    std::optional<gfx::IndexBuffer> quadTriangleIndexBuffer;
    std::optional<gfx::IndexBuffer> tileBorderIndexBuffer;

    static gfx::VertexVector<gfx::Vertex<PositionOnlyLayoutAttributes>> tileVertices();
    static gfx::IndexVector<gfx::Triangles> quadTriangleIndices();
    static gfx::IndexVector<gfx::LineStrip> tileLineStripIndices();
    static gfx::VertexVector<RasterLayoutVertex> rasterVertices();
    static gfx::VertexVector<HeatmapTextureLayoutVertex> heatmapTextureVertices();

    static SegmentVector tileTriangleSegments();
    static SegmentVector tileBorderSegments();
    static SegmentVector rasterSegments();
    static SegmentVector heatmapTextureSegments();

    std::optional<gfx::Renderbuffer<gfx::RenderbufferPixelType::Depth>> depthRenderbuffer;
    bool has3D = false;
    bool uploaded = false;
    Size backendSize;

    std::unique_ptr<gfx::ShaderRegistry> shaders;

    const SegmentVector clippingMaskSegments;
};

} // namespace mbgl
