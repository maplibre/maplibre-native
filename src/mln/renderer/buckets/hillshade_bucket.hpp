#pragma once

#include <mln/gfx/index_buffer.hpp>
#include <mln/renderer/paint_property_binder.hpp>
#include <mln/gfx/vertex_buffer.hpp>
#include <mln/geometry/dem_data.hpp>
#include <mln/shaders/segment.hpp>
#include <mln/renderer/bucket.hpp>
#include <mln/renderer/tile_mask.hpp>
#include <mln/style/layers/hillshade_layer_properties.hpp>
#include <mln/util/tileset.hpp>
#include <mln/util/image.hpp>
#include <mln/util/mat4.hpp>

namespace mln {

using HillshadeBinders = PaintPropertyBinders<style::HillshadePaintProperties::DataDrivenProperties>;
using HillshadeLayoutVertex = gfx::Vertex<TypeList<attributes::pos, attributes::texture_pos>>;

class HillshadeBucket final : public Bucket {
public:
    HillshadeBucket(PremultipliedImage&&, Tileset::RasterEncoding encoding);
    HillshadeBucket(std::shared_ptr<PremultipliedImage>, Tileset::RasterEncoding encoding);
    HillshadeBucket(DEMData&&);
    ~HillshadeBucket() override;

    void upload(gfx::UploadPass&) override;
    bool hasData() const override;

    void clear();
    void setMask(TileMask&&);

    RenderTargetPtr renderTarget;
    bool renderTargetPrepared = false;

    TileMask mask{{0, 0, 0}};

    const DEMData& getDEMData() const;
    DEMData& getDEMData();

    bool isPrepared() const { return prepared; }

    void setPrepared(bool preparedState) { prepared = preparedState; }

    static HillshadeLayoutVertex layoutVertex(Point<int16_t> p, Point<uint16_t> t) {
        return HillshadeLayoutVertex{{{p.x, p.y}}, {{t.x, t.y}}};
    }

    // Raster-DEM Tile Sources use the default buffers from Painter
    using VertexVector = gfx::VertexVector<HillshadeLayoutVertex>;
    std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using IndexVector = gfx::IndexVector<gfx::Triangles>;
    std::shared_ptr<IndexVector> sharedIndices = std::make_shared<IndexVector>();
    IndexVector& indices = *sharedIndices;

    SegmentVector segments;

private:
    DEMData demdata;
    bool prepared = false;
};

} // namespace mln
