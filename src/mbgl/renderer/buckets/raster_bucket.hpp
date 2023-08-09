#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/programs/raster_program.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/tile_mask.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/mat4.hpp>

#include <optional>

namespace mbgl {

class RasterBucket final : public Bucket {
public:
    RasterBucket(PremultipliedImage&&);
    RasterBucket(std::shared_ptr<PremultipliedImage>);
    ~RasterBucket() override;

    void upload(gfx::UploadPass&) override;
    bool hasData() const override;

    void clear();
    void setImage(std::shared_ptr<PremultipliedImage>);
    void setMask(TileMask&&);

    std::shared_ptr<PremultipliedImage> image;
    std::optional<gfx::Texture> texture;
    TileMask mask{{0, 0, 0}};

    // Bucket specific vertices are used for Image Sources only
    // Raster Tile Sources use the default buffers from Painter
    using VertexVector = gfx::VertexVector<RasterLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& indices = *sharedTriangles;

    SegmentVector<RasterAttributes> segments;

#if MLN_LEGACY_RENDERER
    std::optional<gfx::VertexBuffer<RasterLayoutVertex>> vertexBuffer;
    std::optional<gfx::IndexBuffer> indexBuffer;
#endif // MLN_LEGACY_RENDERER
};

} // namespace mbgl
