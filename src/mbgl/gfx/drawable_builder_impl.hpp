#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/polyline_generator.hpp>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>

namespace mbgl {
namespace gfx {

class DrawableBuilder::Impl {
public:
    using VT = gfx::detail::VertexType<gfx::AttributeType<std::int16_t, 2>>;
    gfx::VertexVector<VT> vertices;

    std::vector<uint8_t> rawVertices;
    std::size_t rawVerticesCount = 0;

    struct LineLayoutVertex {
        std::array<int16_t, 2> a1;
        std::array<uint8_t, 4> a2;
    };
    gfx::VertexVector<LineLayoutVertex> polylineVertices;
    gfx::IndexVector<gfx::Triangles> polylineIndexes;

    std::vector<uint16_t> buildIndexes;
    std::shared_ptr<gfx::IndexVectorBase> sharedIndexes;
    std::vector<std::unique_ptr<Drawable::DrawSegment>> segments;

    AttributeDataType rawVerticesType = static_cast<AttributeDataType>(-1);
    gfx::ColorMode colorMode = gfx::ColorMode::disabled();
    gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();

    VertexAttributeArray vertexAttrs;

    void addPolyline(gfx::DrawableBuilder& builder,
                     const GeometryCoordinates& coordinates,
                     const gfx::PolylineGeneratorOptions& options);

    void setupForPolylines(gfx::DrawableBuilder& builder);

private:
    LineLayoutVertex layoutVertex(
        Point<int16_t> p, Point<double> e, bool round, bool up, int8_t dir, int32_t linesofar = 0);
};

} // namespace gfx
} // namespace mbgl
