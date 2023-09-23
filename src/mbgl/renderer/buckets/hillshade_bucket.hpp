#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/programs/hillshade_program.hpp>
#include <mbgl/programs/hillshade_prepare_program.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/tile_mask.hpp>
#include <mbgl/geometry/dem_data.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/mat4.hpp>

namespace mbgl {

class HillshadeBucket final : public Bucket {
public:
    HillshadeBucket(PremultipliedImage&&, Tileset::DEMEncoding encoding);
    HillshadeBucket(std::shared_ptr<PremultipliedImage>, Tileset::DEMEncoding encoding);
    HillshadeBucket(DEMData&&);
    ~HillshadeBucket() override;

    void upload(gfx::UploadPass&) override;
    bool hasData() const override;

    void clear();
    void setMask(TileMask&&);

    std::optional<gfx::Texture> dem;
    std::optional<gfx::Texture> texture;

#if MLN_DRAWABLE_RENDERER
    RenderTargetPtr renderTarget;
    bool renderTargetPrepared = false;
#endif

    TileMask mask{{0, 0, 0}};

    const DEMData& getDEMData() const;
    DEMData& getDEMData();

    bool isPrepared() const { return prepared; }

    void setPrepared(bool preparedState) { prepared = preparedState; }

    // Raster-DEM Tile Sources use the default buffers from Painter
    using VertexVector = gfx::VertexVector<HillshadeLayoutVertex>;
    std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    gfx::IndexVector<gfx::Triangles> indices;
    SegmentVector<HillshadeAttributes> segments;

#if MLN_LEGACY_RENDERER
    std::optional<gfx::VertexBuffer<HillshadeLayoutVertex>> vertexBuffer;
    std::optional<gfx::IndexBuffer> indexBuffer;
#endif // MLN_LEGACY_RENDERER

private:
    DEMData demdata;
    bool prepared = false;
};

} // namespace mbgl
