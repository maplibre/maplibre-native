#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/renderer/tile_mask.hpp>
#include <mbgl/style/layers/raster_layer_properties.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/mat4.hpp>

#include <memory>
#include <optional>

namespace mbgl {

namespace gfx {
class Texture2D;
using Texture2DPtr = std::shared_ptr<Texture2D>;
} // namespace gfx

using RasterBinders = PaintPropertyBinders<style::RasterPaintProperties::DataDrivenProperties>;
using RasterLayoutVertex = gfx::Vertex<TypeList<attributes::pos, attributes::texture_pos>>;

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

    static RasterLayoutVertex layoutVertex(Point<int16_t> p, Point<uint16_t> t) {
        return RasterLayoutVertex{{{p.x, p.y}}, {{t.x, t.y}}};
    }

    std::shared_ptr<PremultipliedImage> image;
    std::optional<gfx::Texture> texture;
    gfx::Texture2DPtr texture2d;
    TileMask mask{{0, 0, 0}};

    // Bucket specific vertices are used for Image Sources only
    // Raster Tile Sources use the default buffers from Painter
    using VertexVector = gfx::VertexVector<RasterLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& indices = *sharedTriangles;

    SegmentVector segments;
};

} // namespace mbgl
