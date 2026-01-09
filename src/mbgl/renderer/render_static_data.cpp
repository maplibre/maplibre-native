#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/shaders/program_parameters.hpp>

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
    result.emplace_back(RasterBucket::layoutVertex({0, 0}, {0, 0}));
    result.emplace_back(RasterBucket::layoutVertex({util::EXTENT, 0}, {util::EXTENT, 0}));
    result.emplace_back(RasterBucket::layoutVertex({0, util::EXTENT}, {0, util::EXTENT}));
    result.emplace_back(RasterBucket::layoutVertex({util::EXTENT, util::EXTENT}, {util::EXTENT, util::EXTENT}));
    return result;
}

gfx::VertexVector<HeatmapTextureLayoutVertex> RenderStaticData::heatmapTextureVertices() {
    gfx::VertexVector<HeatmapTextureLayoutVertex> result;
    result.emplace_back(HeatmapBucket::textureVertex({0, 0}));
    result.emplace_back(HeatmapBucket::textureVertex({1, 0}));
    result.emplace_back(HeatmapBucket::textureVertex({0, 1}));
    result.emplace_back(HeatmapBucket::textureVertex({1, 1}));
    return result;
}

RenderStaticData::RenderStaticData(std::unique_ptr<gfx::ShaderRegistry>&& shaders_)
    : shaders(std::move(shaders_)),
      clippingMaskSegments(tileTriangleSegments()) {}

SegmentVector RenderStaticData::tileTriangleSegments() {
    SegmentVector segments;
    segments.emplace_back(0, 0, 4, 6);
    return segments;
}

SegmentVector RenderStaticData::tileBorderSegments() {
    SegmentVector segments;
    segments.emplace_back(0, 0, 4, 5);
    return segments;
}

SegmentVector RenderStaticData::rasterSegments() {
    SegmentVector segments;
    segments.emplace_back(0, 0, 4, 6);
    return segments;
}

SegmentVector RenderStaticData::heatmapTextureSegments() {
    SegmentVector segments;
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
        uploaded = true;
    }
}

} // namespace mbgl
