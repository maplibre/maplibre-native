#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/renderbuffer.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/programs/background_program.hpp>
#include <mbgl/programs/heatmap_texture_program.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/programs/raster_program.hpp>

#include <string>
#include <optional>

namespace mbgl {
namespace gfx {
class Context;
class UploadPass;
} // namespace gfx

class RenderStaticData {
public:
    RenderStaticData(float pixelRatio, std::unique_ptr<gfx::ShaderRegistry>&& shaders_);

    void upload(gfx::UploadPass&);

    std::optional<gfx::VertexBuffer<gfx::Vertex<PositionOnlyLayoutAttributes>>> tileVertexBuffer;
    std::optional<gfx::VertexBuffer<RasterLayoutVertex>> rasterVertexBuffer;
    std::optional<gfx::VertexBuffer<HeatmapTextureLayoutVertex>> heatmapTextureVertexBuffer;

    std::optional<gfx::IndexBuffer> quadTriangleIndexBuffer;
    std::optional<gfx::IndexBuffer> tileBorderIndexBuffer;

    static SegmentVector<BackgroundAttributes> tileTriangleSegments();
    static SegmentVector<DebugAttributes> tileBorderSegments();
    static SegmentVector<RasterAttributes> rasterSegments();
    static SegmentVector<HeatmapTextureAttributes> heatmapTextureSegments();

    std::optional<gfx::Renderbuffer<gfx::RenderbufferPixelType::Depth>> depthRenderbuffer;
    bool has3D = false;
    bool uploaded = false;
    Size backendSize;

    // @TODO: Migrate away from and remove `Programs`
    Programs programs;

    std::unique_ptr<gfx::ShaderRegistry> shaders;

    const SegmentVector<BackgroundAttributes> clippingMaskSegments;

#ifndef NDEBUG
    Programs overdrawPrograms;
#endif
};

} // namespace mbgl
