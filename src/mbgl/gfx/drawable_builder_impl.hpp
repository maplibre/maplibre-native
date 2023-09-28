#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/style/types.hpp>

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

    std::vector<uint16_t> buildIndexes;
    std::shared_ptr<gfx::IndexVectorBase> sharedIndexes;
    std::vector<std::unique_ptr<Drawable::DrawSegment>> segments;

    AttributeDataType rawVerticesType = static_cast<AttributeDataType>(-1);
    gfx::ColorMode colorMode = gfx::ColorMode::disabled();
    gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();

    VertexAttributeArray vertexAttrs;

    struct PolylineOptions {
        FeatureType type{FeatureType::LineString};
        style::LineJoinType joinType{style::LineJoinType::Miter};
        float miterLimit{2.f};
        style::LineCapType beginCap{style::LineCapType::Butt};
        style::LineCapType endCap{style::LineCapType::Butt};
        float roundLimit{1.f};
        uint32_t overscaling{1}; // TODO: what is this???
    };

    void addPolyline(gfx::DrawableBuilder& builder, const GeometryCoordinates& coordinates, const PolylineOptions& options);

private:
    class TriangleElement;
    class Distances;

    std::ptrdiff_t e1;
    std::ptrdiff_t e2;
    std::ptrdiff_t e3;
    void addCurrentVertex(const GeometryCoordinate& currentCoordinate,
                          double& distance,
                          const Point<double>& normal,
                          double endLeft,
                          double endRight,
                          bool round,
                          std::size_t startVertex,
                          std::vector<TriangleElement>& triangleStore,
                          std::optional<Distances> lineDistances);
    void addPieSliceVertex(const GeometryCoordinate& currentVertex,
                           double distance,
                           const Point<double>& extrude,
                           bool lineTurnsLeft,
                           std::size_t startVertex,
                           std::vector<TriangleElement>& triangleStore,
                           std::optional<Distances> lineDistances);
};

} // namespace gfx
} // namespace mbgl
