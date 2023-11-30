#include <mbgl/gfx/drawable_builder_impl.hpp>

#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/gfx/polyline_generator.hpp>
#include <mbgl/util/logging.hpp>

#include <string>

namespace mbgl {
namespace gfx {

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
                                        const gfx::PolylineGenerator::Options& options) {
    using namespace std::placeholders;
    gfx::PolylineGenerator::generate(
        polylineVertices,
        std::bind(&DrawableBuilder::Impl::layoutVertex, this, _1, _2, _3, _4, _5, _6),
        segments,
        [&builder](std::size_t vertexOffset, std::size_t indexOffset) -> std::unique_ptr<Drawable::DrawSegment> {
            return builder.createSegment(gfx::Triangles(), SegmentBase(vertexOffset, indexOffset));
        },
        [](auto& uniqueSegment) -> SegmentBase& { return uniqueSegment->getSegment(); },
        polylineIndexes,
        coordinates,
        options);
}

void DrawableBuilder::Impl::setupForPolylines(gfx::Context& context, gfx::DrawableBuilder& builder) {
    // setup vertex attributes
    static const StringIdentity idVertexAttribName = stringIndexer().get("a_pos_normal");
    static const StringIdentity idDataAttribName = stringIndexer().get("a_data");

    builder.setVertexAttrNameId(idVertexAttribName);

    builder.setRawVertices({}, polylineVertices.elements(), gfx::AttributeDataType::Short2);

    auto attrs = context.createVertexAttributeArray();
    using VertexVector = gfx::VertexVector<LineLayoutVertex>;
    std::shared_ptr<VertexVector> verts = std::make_shared<VertexVector>(std::move(polylineVertices));

    if (const auto& attr = attrs->add(idVertexAttribName)) {
        attr->setSharedRawData(verts,
                               offsetof(LineLayoutVertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(LineLayoutVertex),
                               gfx::AttributeDataType::Short2);
    }

    if (const auto& attr = attrs->add(idDataAttribName)) {
        attr->setSharedRawData(verts,
                               offsetof(LineLayoutVertex, a2),
                               /*vertexOffset=*/0,
                               sizeof(LineLayoutVertex),
                               gfx::AttributeDataType::UByte4);
    }

    builder.setVertexAttributes(std::move(attrs));

    sharedIndexes = std::make_shared<gfx::IndexVectorBase>(std::move(polylineIndexes));
}

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
