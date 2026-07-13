#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/shaders/program_parameters.hpp>

namespace mbgl {

RenderStaticData::RenderStaticData(std::unique_ptr<gfx::ShaderRegistry>&& shaders_)
    : shaders(std::move(shaders_)),
      clippingMaskSegments(tileTriangleSegments()) {}

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

gfx::VertexVector<gfx::Vertex<PositionOnlyLayoutAttributes>> RenderStaticData::tileVertices() {
    gfx::VertexVector<gfx::Vertex<PositionOnlyLayoutAttributes>> vertices;
    vertices.emplace_back(gfx::Vertex<PositionOnlyLayoutAttributes>({{{0, 0}}}));
    vertices.emplace_back(gfx::Vertex<PositionOnlyLayoutAttributes>({{{util::EXTENT, 0}}}));
    vertices.emplace_back(gfx::Vertex<PositionOnlyLayoutAttributes>({{{0, util::EXTENT}}}));
    vertices.emplace_back(gfx::Vertex<PositionOnlyLayoutAttributes>({{{util::EXTENT, util::EXTENT}}}));
    return vertices;
}

gfx::VertexVector<RasterLayoutVertex> RenderStaticData::rasterVertices() {
    gfx::VertexVector<RasterLayoutVertex> vertices;
    vertices.emplace_back(RasterBucket::layoutVertex({0, 0}, {0, 0}));
    vertices.emplace_back(RasterBucket::layoutVertex({util::EXTENT, 0}, {util::EXTENT, 0}));
    vertices.emplace_back(RasterBucket::layoutVertex({0, util::EXTENT}, {0, util::EXTENT}));
    vertices.emplace_back(RasterBucket::layoutVertex({util::EXTENT, util::EXTENT}, {util::EXTENT, util::EXTENT}));
    return vertices;
}

gfx::VertexVector<HeatmapTextureLayoutVertex> RenderStaticData::heatmapTextureVertices() {
    gfx::VertexVector<HeatmapTextureLayoutVertex> vertices;
    vertices.emplace_back(HeatmapBucket::textureVertex({0, 0}));
    vertices.emplace_back(HeatmapBucket::textureVertex({1, 0}));
    vertices.emplace_back(HeatmapBucket::textureVertex({0, 1}));
    vertices.emplace_back(HeatmapBucket::textureVertex({1, 1}));
    return vertices;
}

gfx::VertexVector<FillExtrusionStaticVertex> RenderStaticData::fillExtrusionVertices() {
    gfx::VertexVector<HeatmapTextureLayoutVertex> vertices;
    vertices.emplace_back(FillExtrusionStaticVertex{1, 0});
    vertices.emplace_back(FillExtrusionStaticVertex{1, 1});
    vertices.emplace_back(FillExtrusionStaticVertex{0, 0});
    vertices.emplace_back(FillExtrusionStaticVertex{0, 1});
    return vertices;
}

gfx::IndexVector<gfx::Triangles> RenderStaticData::quadTriangleIndices() {
    gfx::IndexVector<gfx::Triangles> indices;
    indices.emplace_back(0, 1, 2);
    indices.emplace_back(1, 2, 3);
    return indices;
}

gfx::IndexVector<gfx::LineStrip> RenderStaticData::tileLineStripIndices() {
    gfx::IndexVector<gfx::LineStrip> indices;
    indices.emplace_back(0);
    indices.emplace_back(1);
    indices.emplace_back(3);
    indices.emplace_back(2);
    indices.emplace_back(0);
    return indices;
}

gfx::IndexVector<gfx::Triangles> RenderStaticData::fillExtrusionTriangleIndices() {
    gfx::IndexVector<gfx::Triangles> indices;
    // ┌──────┐
    // │ 0  1 │ Counter-Clockwise winding order.
    // │      │ Triangle 1: 0 => 2 => 1
    // │ 2  3 │ Triangle 2: 1 => 2 => 3
    // └──────┘
    indices.emplace_back(0, 2, 1);
    indices.emplace_back(1, 2, 3);
    return indices;
}

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

SegmentVector RenderStaticData::fillExtrusionSegments() {
    SegmentVector segments;
    segments.emplace_back(0, 0, 4, 6);
    return segments;
}

} // namespace mbgl
