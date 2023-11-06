#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/programs/program_parameters.hpp>

namespace mbgl {

gfx::VertexVector<gfx::Vertex<PositionOnlyLayoutAttributes>> RenderStaticData::tileVertices() {
    gfx::VertexVector<gfx::Vertex<PositionOnlyLayoutAttributes>> result;
    result.emplace_back(gfx::Vertex<PositionOnlyLayoutAttributes>({{{0, 0}}}));
    result.emplace_back(gfx::Vertex<PositionOnlyLayoutAttributes>({{{util::EXTENT, 0}}}));
    result.emplace_back(gfx::Vertex<PositionOnlyLayoutAttributes>({{{0, util::EXTENT}}}));
    result.emplace_back(gfx::Vertex<PositionOnlyLayoutAttributes>({{{util::EXTENT, util::EXTENT}}}));
    return result;
}

gfx::IndexVector<gfx::Triangles> RenderStaticData::quadTriangleIndices() {
    gfx::IndexVector<gfx::Triangles> result;
    result.emplace_back(0, 1, 2);
    result.emplace_back(1, 2, 3);
    return result;
}

gfx::IndexVector<gfx::LineStrip> RenderStaticData::tileLineStripIndices() {
    gfx::IndexVector<gfx::LineStrip> result;
    result.emplace_back(0);
    result.emplace_back(1);
    result.emplace_back(3);
    result.emplace_back(2);
    result.emplace_back(0);
    return result;
}

gfx::VertexVector<RasterLayoutVertex> RenderStaticData::rasterVertices() {
    gfx::VertexVector<RasterLayoutVertex> result;
    result.emplace_back(RasterProgram::layoutVertex({0, 0}, {0, 0}));
    result.emplace_back(RasterProgram::layoutVertex({util::EXTENT, 0}, {util::EXTENT, 0}));
    result.emplace_back(RasterProgram::layoutVertex({0, util::EXTENT}, {0, util::EXTENT}));
    result.emplace_back(RasterProgram::layoutVertex({util::EXTENT, util::EXTENT}, {util::EXTENT, util::EXTENT}));
    return result;
}

gfx::VertexVector<HeatmapTextureLayoutVertex> RenderStaticData::heatmapTextureVertices() {
    gfx::VertexVector<HeatmapTextureLayoutVertex> result;
    result.emplace_back(HeatmapTextureProgram::layoutVertex({0, 0}));
    result.emplace_back(HeatmapTextureProgram::layoutVertex({1, 0}));
    result.emplace_back(HeatmapTextureProgram::layoutVertex({0, 1}));
    result.emplace_back(HeatmapTextureProgram::layoutVertex({1, 1}));
    return result;
}

RenderStaticData::RenderStaticData(float pixelRatio, std::unique_ptr<gfx::ShaderRegistry>&& shaders_)
    : programs(ProgramParameters{pixelRatio, false}),
      shaders(std::move(shaders_)),
      clippingMaskSegments(tileTriangleSegments())
#ifndef NDEBUG
      ,
      overdrawPrograms(ProgramParameters{pixelRatio, true})
#endif
{
}

SegmentVector<BackgroundAttributes> RenderStaticData::tileTriangleSegments() {
    SegmentVector<BackgroundAttributes> segments;
    segments.emplace_back(0, 0, 4, 6);
    return segments;
}

SegmentVector<DebugAttributes> RenderStaticData::tileBorderSegments() {
    SegmentVector<DebugAttributes> segments;
    segments.emplace_back(0, 0, 4, 5);
    return segments;
}

SegmentVector<RasterAttributes> RenderStaticData::rasterSegments() {
    SegmentVector<RasterAttributes> segments;
    segments.emplace_back(0, 0, 4, 6);
    return segments;
}

SegmentVector<HeatmapTextureAttributes> RenderStaticData::heatmapTextureSegments() {
    SegmentVector<HeatmapTextureAttributes> segments;
    segments.emplace_back(0, 0, 4, 6);
    return segments;
}

void RenderStaticData::upload(gfx::UploadPass& uploadPass) {
    if (!uploaded) {
        // these are still used by stencil buffer rendering
        tileVertexBuffer = uploadPass.createVertexBuffer(
            tileVertices(), gfx::BufferUsageType::StaticDraw, /*persistent=*/false);
        quadTriangleIndexBuffer = uploadPass.createIndexBuffer(
            quadTriangleIndices(), gfx::BufferUsageType::StaticDraw, /*persistent=*/false);
#if MLN_LEGACY_RENDERER
        rasterVertexBuffer = uploadPass.createVertexBuffer(rasterVertices());
        heatmapTextureVertexBuffer = uploadPass.createVertexBuffer(heatmapTextureVertices());
        tileBorderIndexBuffer = uploadPass.createIndexBuffer(tileLineStripIndices());
#endif // MLN_LEGACY_RENDERER
        uploaded = true;
    }
}

} // namespace mbgl
