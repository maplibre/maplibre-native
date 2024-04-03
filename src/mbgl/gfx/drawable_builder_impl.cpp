#include <mbgl/gfx/drawable_builder_impl.hpp>

#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/polyline_generator.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/projection.hpp>

#include <string>

namespace mbgl {
namespace gfx {

using namespace shaders;

#pragma mark Polylines

DrawableBuilder::Impl::LineLayoutVertex DrawableBuilder::Impl::layoutVertex(
    Point<int16_t> p, Point<double> e, bool round, bool up, int8_t dir, int32_t linesofar /*= 0*/) {
    /*
     * Scale the extrusion vector so that the normal length is this value.
     * Contains the "texture" normals (-1..1). This is distinct from the extrude
     * normals for line joins, because the x-value remains 0 for the texture
     * normal array, while the extrude normal actually moves the vertex to
     * create the acute/bevelled line join.
     */
    static const int8_t extrudeScale = 63;
    return LineLayoutVertex{
        {{static_cast<int16_t>((p.x * 2) | (round ? 1 : 0)), static_cast<int16_t>((p.y * 2) | (up ? 1 : 0))}},
        {{// add 128 to store a byte in an unsigned byte
          static_cast<uint8_t>(::round(extrudeScale * e.x) + 128),
          static_cast<uint8_t>(::round(extrudeScale * e.y) + 128),

          // Encode the -1/0/1 direction value into the first two bits of .z
          // of a_data. Combine it with the lower 6 bits of `linesofar`
          // (shifted by 2 bites to make room for the direction value). The
          // upper 8 bits of `linesofar` are placed in the `w` component.
          // `linesofar` is scaled down by `LINE_DISTANCE_SCALE` so that we
          // can store longer distances while sacrificing precision.

          // Encode the -1/0/1 direction value into .zw coordinates of
          // a_data, which is normally covered by linesofar, so we need to
          // merge them. The z component's first bit, as well as the sign
          // bit is reserved for the direction, so we need to shift the
          // linesofar.
          static_cast<uint8_t>(((dir == 0 ? 0 : (dir < 0 ? -1 : 1)) + 1) | ((linesofar & 0x3F) << 2)),
          static_cast<uint8_t>(linesofar >> 6)}}};
}

void DrawableBuilder::Impl::addPolyline(gfx::DrawableBuilder& builder,
                                        const GeometryCoordinates& coordinates,
                                        const gfx::PolylineGeneratorOptions& options) {
    using namespace std::placeholders;
    gfx::PolylineGenerator<DrawableBuilder::Impl::LineLayoutVertex, std::unique_ptr<Drawable::DrawSegment>> generator(
        polylineVertices,
        std::bind(&DrawableBuilder::Impl::layoutVertex, this, _1, _2, _3, _4, _5, _6),
        segments,
        [&builder](std::size_t vertexOffset, std::size_t indexOffset) -> std::unique_ptr<Drawable::DrawSegment> {
            return builder.createSegment(gfx::Triangles(), SegmentBase(vertexOffset, indexOffset));
        },
        [](auto& uniqueSegment) -> SegmentBase& { return uniqueSegment->getSegment(); },
        polylineIndexes);
    generator.generate(coordinates, options);
}

void DrawableBuilder::Impl::setupForPolylines(gfx::Context& context, gfx::DrawableBuilder& builder) {
    // setup vertex attributes
    builder.setVertexAttrId(idLinePosNormalVertexAttribute);
    builder.setRawVertices({}, polylineVertices.elements(), gfx::AttributeDataType::Short2);

    auto attrs = context.createVertexAttributeArray();
    using VertexVector = gfx::VertexVector<LineLayoutVertex>;
    std::shared_ptr<VertexVector> verts = std::make_shared<VertexVector>(std::move(polylineVertices));

    if (const auto& attr = attrs->set(idLinePosNormalVertexAttribute)) {
        attr->setSharedRawData(verts,
                               offsetof(LineLayoutVertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(LineLayoutVertex),
                               gfx::AttributeDataType::Short2);
    }

    if (const auto& attr = attrs->set(idLineDataVertexAttribute)) {
        attr->setSharedRawData(verts,
                               offsetof(LineLayoutVertex, a2),
                               /*vertexOffset=*/0,
                               sizeof(LineLayoutVertex),
                               gfx::AttributeDataType::UByte4);
    }

    builder.setVertexAttributes(std::move(attrs));

    sharedIndexes = std::make_shared<gfx::IndexVectorBase>(std::move(polylineIndexes));
}

#pragma mark Wide Vector Polylines

void DrawableBuilder::Impl::addWideVectorPolylineLocal(gfx::DrawableBuilder& /*builder*/,
                                                       const GeometryCoordinates& coordinates,
                                                       const gfx::PolylineGeneratorOptions& options) {
    // TODO: implement appending to existing polyline
    // add instance data
    int index = 0;
    const int coord_size = static_cast<int>(coordinates.size());
    for (const auto& coord : coordinates) {
        VertexTriWideVecInstance data;
        data.center = {static_cast<float>(coord.x), static_cast<float>(coord.y), 0};

        if (FeatureType::Polygon == options.type) {
            // loop line
            data.prev = (index - 1 + coord_size) % coord_size;
            data.next = (index + 1) % coord_size;
        } else {
            // line string
            assert(FeatureType::LineString == options.type);
            data.prev = index - 1;
            data.next = index + 1 >= coord_size ? -1 : index + 1;
        }

        wideVectorInstanceData.emplace_back(data);
        ++index;
    }
}

mbgl::Point<double> DrawableBuilder::Impl::addWideVectorPolylineGlobal(gfx::DrawableBuilder& /*builder*/,
                                                                       const LineString<double>& coordinates,
                                                                       const gfx::PolylineGeneratorOptions& options) {
    // TODO: implement appending to existing polyline

    constexpr int32_t zoom = 0;

    // get center
    constexpr double maxd = std::numeric_limits<double>::max();
    constexpr double mind = std::numeric_limits<double>::min();
    Point<double> minPoint{maxd, maxd}, maxPoint{mind, mind};
    for (const auto& coord : coordinates) {
        auto merc = Projection::project(LatLng(coord.y, coord.x), zoom);
        Point<double> pSource{merc.x * mbgl::util::EXTENT, merc.y * mbgl::util::EXTENT};
        minPoint.x = std::min(pSource.x, minPoint.x);
        minPoint.y = std::min(pSource.y, minPoint.y);
        maxPoint.x = std::max(pSource.x, maxPoint.x);
        maxPoint.y = std::max(pSource.y, maxPoint.y);
    }
    Point<double> pCenter{(minPoint.x + maxPoint.x) / 2.0, (minPoint.y + maxPoint.y) / 2.0};

    // add centerline instance data
    int index = 0;
    const int coord_size = static_cast<int>(coordinates.size());
    for (const auto& coord : coordinates) {
        auto merc = Projection::project(LatLng(coord.y, coord.x), zoom);
        Point<double> pSource{merc.x * mbgl::util::EXTENT, merc.y * mbgl::util::EXTENT};
        pSource.x -= pCenter.x;
        pSource.y -= pCenter.y;
        VertexTriWideVecInstance data;
        data.center = {static_cast<float>(pSource.x), static_cast<float>(pSource.y), 0};

        if (FeatureType::Polygon == options.type) {
            // loop line
            data.prev = (index - 1 + coord_size) % coord_size;
            data.next = (index + 1) % coord_size;
        } else {
            // line string
            assert(FeatureType::LineString == options.type);
            data.prev = index - 1;
            data.next = index + 1 >= coord_size ? -1 : index + 1;
        }

        wideVectorInstanceData.emplace_back(data);
        ++index;
    }

    return pCenter;
}

void DrawableBuilder::Impl::setupForWideVectors(gfx::Context& context, gfx::DrawableBuilder& builder) {
    // vertices
    constexpr shaders::VertexTriWideVecB wideVectorVertices[]{
        {{0}, {0}, (0 << 16) + 0},
        {{0}, {0}, (1 << 16) + 0},
        {{0}, {0}, (2 << 16) + 0},
        {{0}, {0}, (3 << 16) + 0},

        {{0}, {0}, (4 << 16) + 1},
        {{0}, {0}, (5 << 16) + 1},
        {{0}, {0}, (6 << 16) + 1},
        {{0}, {0}, (7 << 16) + 1},

        {{0}, {0}, (8 << 16) + 2},
        {{0}, {0}, (9 << 16) + 2},
        {{0}, {0}, (10 << 16) + 2},
        {{0}, {0}, (11 << 16) + 2},
    };

    using VertexVector = gfx::VertexVector<VertexTriWideVecB>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices_ = *sharedVertices;
    for (const auto& v : wideVectorVertices) {
        vertices_.emplace_back(v);
    }

    // instance data
    using InstanceVector = gfx::VertexVector<VertexTriWideVecInstance>;
    const std::shared_ptr<InstanceVector> sharedInstanceData = std::make_shared<InstanceVector>(
        std::move(wideVectorInstanceData));

    // indexes
    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;
    triangles.emplace_back(
        0, 3, 1, 0, 2, 3, 4 + 0, 4 + 3, 4 + 1, 4 + 0, 4 + 2, 4 + 3, 8 + 0, 8 + 3, 8 + 1, 8 + 0, 8 + 2, 8 + 3);

    // segments
    SegmentVector<VertexTriWideVecB> triangleSegments;
    triangleSegments.emplace_back(Segment<VertexTriWideVecB>{0, 0, 12, 18});

    // add to builder
    {
        // add vertex attributes
        auto vertexAttributes = context.createVertexAttributeArray();
        if (const auto& attr = vertexAttributes->set(idWideVectorScreenPos)) {
            attr->setSharedRawData(sharedVertices,
                                   offsetof(VertexTriWideVecB, screenPos),
                                   /*vertexOffset=*/0,
                                   sizeof(VertexTriWideVecB),
                                   gfx::AttributeDataType::Float3);
        }
        if (const auto& attr = vertexAttributes->set(idWideVectorColor)) {
            attr->setSharedRawData(sharedVertices,
                                   offsetof(VertexTriWideVecB, color),
                                   /*vertexOffset=*/0,
                                   sizeof(VertexTriWideVecB),
                                   gfx::AttributeDataType::Float4);
        }
        if (const auto& attr = vertexAttributes->set(idWideVectorIndex)) {
            attr->setSharedRawData(sharedVertices,
                                   offsetof(VertexTriWideVecB, index),
                                   /*vertexOffset=*/0,
                                   sizeof(VertexTriWideVecB),
                                   gfx::AttributeDataType::Int);
        }
        builder.setVertexAttributes(std::move(vertexAttributes));
    }

    {
        // add instance attributes
        auto instanceAttributes = context.createVertexAttributeArray();
        if (const auto& attr = instanceAttributes->set(idWideVectorInstanceCenter)) {
            attr->setSharedRawData(sharedInstanceData,
                                   offsetof(VertexTriWideVecInstance, center),
                                   /*vertexOffset=*/0,
                                   sizeof(VertexTriWideVecInstance),
                                   gfx::AttributeDataType::Float3);
        }
        if (const auto& attr = instanceAttributes->set(idWideVectorInstanceColor)) {
            attr->setSharedRawData(sharedInstanceData,
                                   offsetof(VertexTriWideVecInstance, color),
                                   /*vertexOffset=*/0,
                                   sizeof(VertexTriWideVecInstance),
                                   gfx::AttributeDataType::Float4);
        }
        if (const auto& attr = instanceAttributes->set(idWideVectorInstancePrevious)) {
            attr->setSharedRawData(sharedInstanceData,
                                   offsetof(VertexTriWideVecInstance, prev),
                                   /*vertexOffset=*/0,
                                   sizeof(VertexTriWideVecInstance),
                                   gfx::AttributeDataType::Int);
        }
        if (const auto& attr = instanceAttributes->set(idWideVectorInstanceNext)) {
            attr->setSharedRawData(sharedInstanceData,
                                   offsetof(VertexTriWideVecInstance, next),
                                   /*vertexOffset=*/0,
                                   sizeof(VertexTriWideVecInstance),
                                   gfx::AttributeDataType::Int);
        }
        builder.setInstanceAttributes(std::move(instanceAttributes));
    }

    builder.setRawVertices({}, vertices_.elements(), gfx::AttributeDataType::Float2);
    builder.setSegments(gfx::Triangles(), sharedTriangles, triangleSegments.data(), triangleSegments.size());
}

#pragma mark Common

bool DrawableBuilder::Impl::checkAndSetMode(Mode target) {
    if (target != mode && vertexCount()) {
        // log error
        using namespace std::string_literals;
        auto to_string = [](Mode value) -> std::string {
            switch (value) {
                case Mode::Primitives:
                    return "Mode::Primitives";
                case Mode::Polylines:
                    return "Mode::Polylines";
                case Mode::WideVectorLocal:
                    return "Mode::WideVectorLocal";
                case Mode::WideVectorGlobal:
                    return "Mode::WideVectorGlobal";
                case Mode::Custom:
                    return "Mode::Custom";
                default:
                    return "Unknown"; // Handle unknown enum values if needed
            }
        };
        mbgl::Log::Error(
            mbgl::Event::General,
            "DrawableBuilder mode mismatch. Target is "s + to_string(target) + ", current is " + to_string(mode));

        // the builder is building in a different mode
        assert(false);
        // TODO: throw?
        return false;
    }
    mode = target;
    return true;
}

} // namespace gfx
} // namespace mbgl
