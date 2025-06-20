#include <mbgl/gfx/polyline_generator.hpp>

#include <mbgl/style/types.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/renderer/buckets/line_bucket.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/gfx/drawable_impl.hpp>

#include <memory>
#include <numbers>

using namespace std::numbers;

namespace mbgl {
namespace gfx {

namespace {
/*
 * Sharp corners cause dashed lines to tilt because the distance along the line
 * is the same at both the inner and outer corners. To improve the appearance of
 * dashed lines we add extra points near sharp corners so that a smaller part
 * of the line is tilted.
 *
 * COS_HALF_SHARP_CORNER controls how sharp a corner has to be for us to add an
 * extra vertex. The default is 75 degrees.
 *
 * The newly created vertices are placed SHARP_CORNER_OFFSET pixels from the corner.
 */
const float COS_HALF_SHARP_CORNER = std::cos(75.0f / 2.0f * (pi_v<float> / 180.0f));
constexpr float SHARP_CORNER_OFFSET = 15.0f;

// Angle per triangle for approximating round line joins.
constexpr float DEG_PER_TRIANGLE = 20.0f;

// The number of bits that is used to store the line distance in the buffer.
constexpr int LINE_DISTANCE_BUFFER_BITS = 14;

// We don't have enough bits for the line distance as we'd like to have, so
// use this value to scale the line distance (in tile units) down to a smaller
// value. This lets us store longer distances while sacrificing precision.
constexpr float LINE_DISTANCE_SCALE = 1.0 / 2.0;

// The maximum line distance, in tile units, that fits in the buffer.
constexpr auto MAX_LINE_DISTANCE = static_cast<float>((1u << LINE_DISTANCE_BUFFER_BITS) / LINE_DISTANCE_SCALE);

} // namespace

double PolylineGeneratorDistances::scaleToMaxLineDistance(double tileDistance) const {
    double relativeTileDistance = tileDistance / total;
    if (std::isinf(relativeTileDistance) || std::isnan(relativeTileDistance)) {
        assert(false);
        relativeTileDistance = 0.0;
    }
    return (relativeTileDistance * (clipEnd - clipStart) + clipStart) * (MAX_LINE_DISTANCE - 1);
}

template <class PLV, class PS>
struct PolylineGenerator<PLV, PS>::TriangleElement {
    TriangleElement(uint16_t a_, uint16_t b_, uint16_t c_)
        : a(a_),
          b(b_),
          c(c_) {}
    uint16_t a, b, c;
};

template <class PLV, class PS>
PolylineGenerator<PLV, PS>::PolylineGenerator(Vertices& polylineVertices,
                                              LayoutVertexFunc layoutVertexFunc,
                                              Segments& polylineSegments,
                                              CreateSegmentFunc createSegmentFunc,
                                              GetSegmentFunc getSegmentFunc,
                                              Indexes& polylineIndexes)
    : vertices(polylineVertices),
      layoutVertex(layoutVertexFunc),
      segments(polylineSegments),
      createSegment(createSegmentFunc),
      getSegment(getSegmentFunc),
      indexes(polylineIndexes) {}

template <class PLV, class PS>
void PolylineGenerator<PLV, PS>::generate(const GeometryCoordinates& coordinates,
                                          const PolylineGeneratorOptions& options) {
    const std::size_t len = [&coordinates] {
        std::size_t l = coordinates.size();
        // If the line has duplicate vertices at the end, adjust length to remove them.
        while (l >= 2 && coordinates[l - 1] == coordinates[l - 2]) {
            l--;
        }
        return l;
    }();

    const std::size_t first = [&coordinates, &len] {
        std::size_t i = 0;
        // If the line has duplicate vertices at the start, adjust index to remove them.
        while (i < len - 1 && coordinates[i] == coordinates[i + 1]) {
            i++;
        }
        return i;
    }();

    // Ignore invalid geometry.
    if (len < (options.type == FeatureType::Polygon ? 3 : 2)) {
        return;
    }

    const style::LineJoinType joinType = options.joinType;

    const float miterLimit = joinType == style::LineJoinType::Bevel ? 1.05f : options.miterLimit;

    const uint32_t overscaling = options.overscaling;

    const double sharpCornerOffset =
        overscaling == 0
            ? SHARP_CORNER_OFFSET * (util::EXTENT / util::tileSize_D)
            : (overscaling <= 16.0 ? SHARP_CORNER_OFFSET * (util::EXTENT / (util::tileSize_D * overscaling)) : 0.0);

    const GeometryCoordinate firstCoordinate = coordinates[first];
    const style::LineCapType beginCap = options.beginCap;
    const style::LineCapType endCap = options.type == FeatureType::Polygon ? style::LineCapType::Butt : options.endCap;

    double distance = 0.0;
    bool startOfLine = true;
    std::optional<GeometryCoordinate> currentCoordinate;
    std::optional<GeometryCoordinate> prevCoordinate;
    std::optional<GeometryCoordinate> nextCoordinate;
    std::optional<Point<double>> prevNormal;
    std::optional<Point<double>> nextNormal;

    // the last three vertices added
    e1 = e2 = e3 = -1;

    if (options.type == FeatureType::Polygon) {
        currentCoordinate = coordinates[len - 2];
        nextNormal = util::perp(util::unit(convertPoint<double>(firstCoordinate - *currentCoordinate)));
    }

    const std::size_t startVertex = vertices.elements();
    std::vector<TriangleElement> triangleStore;

    // Pre-allocate for triangles based on measuring benchmark execution
    constexpr auto approxTrianglesPerSegment = 6;
    triangleStore.reserve((len - first) * approxTrianglesPerSegment);

    // Vertex count depends on length rather than segment count, and two elements often generates
    // thousands of vertices, so we allocate some extra memory to skip the next 10 allocations.
    if (vertices.empty()) {
        vertices.reserve(1 << 10);
    }

    for (std::size_t i = first; i < len; ++i) {
        if (options.type == FeatureType::Polygon && i == len - 1) {
            // if the line is closed, we treat the last vertex like the first
            nextCoordinate = coordinates[first + 1];
        } else if (i + 1 < len) {
            // just the next vertex
            nextCoordinate = coordinates[i + 1];
        } else {
            // there is no next vertex
            nextCoordinate = {};
        }

        // if two consecutive vertices exist, skip the current one
        if (nextCoordinate && coordinates[i] == *nextCoordinate) {
            continue;
        }

        if (nextNormal) {
            prevNormal = *nextNormal;
        }
        if (currentCoordinate) {
            prevCoordinate = *currentCoordinate;
        }

        currentCoordinate = coordinates[i];

        // Calculate the normal towards the next vertex in this line. In case
        // there is no next vertex, pretend that the line is continuing
        // straight, meaning that we are just using the previous normal.
        nextNormal = nextCoordinate ? util::perp(util::unit(convertPoint<double>(*nextCoordinate - *currentCoordinate)))
                                    : prevNormal;

        // If we still don't have a previous normal, this is the beginning of a
        // non-closed line, so we're doing a straight "join".
        if (!prevNormal) {
            prevNormal = *nextNormal;
        }

        // Determine the normal of the join extrusion. It is the angle bisector
        // of the segments between the previous line and the next line.
        // In the case of 180° angles, the prev and next normals cancel each
        // other out: prevNormal + nextNormal = (0, 0), its magnitude is 0, so
        // the unit vector would be undefined. In that case, we're keeping the
        // joinNormal at (0, 0), so that the cosHalfAngle below will also become
        // 0 and miterLength will become Infinity.
        Point<double> joinNormal = *prevNormal + *nextNormal;
        if (joinNormal.x != 0 || joinNormal.y != 0) {
            joinNormal = util::unit(joinNormal);
        }

        // *  joinNormal     prevNormal
        // *             ↖      ↑
        // *                .________. prevVertex
        // *                |
        // * nextNormal  ←  |  currentVertex
        // *                |
        // *     nextVertex !
        // *
        //

        // Calculate cosines of the angle (and its half) using dot product.
        const double cosAngle = prevNormal->x * nextNormal->x + prevNormal->y * nextNormal->y;
        const double cosHalfAngle = joinNormal.x * nextNormal->x + joinNormal.y * nextNormal->y;

        // Calculate the length of the miter (the ratio of the miter to the width)
        // as the inverse of cosine of the angle between next and join normals.
        const double miterLength = cosHalfAngle != 0 ? 1 / cosHalfAngle : std::numeric_limits<double>::infinity();

        // Approximate angle from cosine.
        const double approxAngle = 2 * std::sqrt(2 - 2 * cosHalfAngle);

        const bool isSharpCorner = cosHalfAngle < COS_HALF_SHARP_CORNER && prevCoordinate && nextCoordinate;

        if (isSharpCorner && i > first) {
            const auto prevSegmentLength = util::dist<double>(*currentCoordinate, *prevCoordinate);
            if (prevSegmentLength > 2.0 * sharpCornerOffset) {
                GeometryCoordinate newPrevVertex = *currentCoordinate -
                                                   convertPoint<int16_t>(util::round(
                                                       convertPoint<double>(*currentCoordinate - *prevCoordinate) *
                                                       (sharpCornerOffset / prevSegmentLength)));
                distance += util::dist<double>(newPrevVertex, *prevCoordinate);
                addCurrentVertex(newPrevVertex,
                                 distance,
                                 *prevNormal,
                                 0,
                                 0,
                                 false,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);
                prevCoordinate = newPrevVertex;
            }
        }

        // The join if a middle vertex, otherwise the cap
        const bool middleVertex = prevCoordinate && nextCoordinate;
        style::LineJoinType currentJoin = joinType;
        const style::LineCapType currentCap = nextCoordinate ? beginCap : endCap;

        if (middleVertex) {
            if (currentJoin == style::LineJoinType::Round) {
                if (miterLength < options.roundLimit) {
                    currentJoin = style::LineJoinType::Miter;
                } else if (miterLength <= 2) {
                    currentJoin = style::LineJoinType::FakeRound;
                }
            }

            if (currentJoin == style::LineJoinType::Miter && miterLength > miterLimit) {
                currentJoin = style::LineJoinType::Bevel;
            }

            if (currentJoin == style::LineJoinType::Bevel) {
                // The maximum extrude length is 128 / 63 = 2 times the width of
                // the line so if miterLength >= 2 we need to draw a different
                // type of bevel here.
                if (miterLength > 2) {
                    currentJoin = style::LineJoinType::FlipBevel;
                }

                // If the miterLength is really small and the line bevel wouldn't be visible,
                // just draw a miter join to save a triangle.
                if (miterLength < miterLimit) {
                    currentJoin = style::LineJoinType::Miter;
                }
            }
        }

        // Calculate how far along the line the currentVertex is
        if (prevCoordinate) distance += util::dist<double>(*currentCoordinate, *prevCoordinate);

        if (middleVertex && currentJoin == style::LineJoinType::Miter) {
            joinNormal = joinNormal * miterLength;
            addCurrentVertex(*currentCoordinate,
                             distance,
                             joinNormal,
                             0,
                             0,
                             false,
                             startVertex,
                             triangleStore,
                             options.clipDistances);

        } else if (middleVertex && currentJoin == style::LineJoinType::FlipBevel) {
            // miter is too big, flip the direction to make a beveled join

            if (miterLength > 100) {
                // Almost parallel lines
                joinNormal = *nextNormal * -1.0;
            } else {
                const double direction = prevNormal->x * nextNormal->y - prevNormal->y * nextNormal->x > 0 ? -1 : 1;
                const double bevelLength = miterLength * util::mag(*prevNormal + *nextNormal) /
                                           util::mag(*prevNormal - *nextNormal);
                joinNormal = util::perp(joinNormal) * bevelLength * direction;
            }

            addCurrentVertex(*currentCoordinate,
                             distance,
                             joinNormal,
                             0,
                             0,
                             false,
                             startVertex,
                             triangleStore,
                             options.clipDistances);

            addCurrentVertex(*currentCoordinate,
                             distance,
                             joinNormal * -1.0,
                             0,
                             0,
                             false,
                             startVertex,
                             triangleStore,
                             options.clipDistances);
        } else if (middleVertex &&
                   (currentJoin == style::LineJoinType::Bevel || currentJoin == style::LineJoinType::FakeRound)) {
            const bool lineTurnsLeft = (prevNormal->x * nextNormal->y - prevNormal->y * nextNormal->x) > 0;
            const auto offset = static_cast<float>(-std::sqrt(miterLength * miterLength - 1));
            float offsetA;
            float offsetB;

            if (lineTurnsLeft) {
                offsetB = 0;
                offsetA = offset;
            } else {
                offsetA = 0;
                offsetB = offset;
            }

            // Close previous segement with bevel
            if (!startOfLine) {
                addCurrentVertex(*currentCoordinate,
                                 distance,
                                 *prevNormal,
                                 offsetA,
                                 offsetB,
                                 false,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);
            }

            if (currentJoin == style::LineJoinType::FakeRound) {
                // The join angle is sharp enough that a round join would be
                // visible. Bevel joins fill the gap between segments with a
                // single pie slice triangle. Create a round join by adding
                // multiple pie slices. The join isn't actually round, but it
                // looks like it is at the sizes we render lines at.

                // Pick the number of triangles for approximating round join by
                // based on the angle between normals.
                const auto n = static_cast<unsigned>(::round((approxAngle * 180 / pi) / DEG_PER_TRIANGLE));

                for (unsigned m = 1; m < n; ++m) {
                    double t = static_cast<double>(m) / n;
                    if (t != 0.5) {
                        // approximate spherical interpolation
                        // https://observablehq.com/@mourner/approximating-geometric-slerp
                        const double t2 = t - 0.5;
                        const double A = 1.0904 + cosAngle * (-3.2452 + cosAngle * (3.55645 - cosAngle * 1.43519));
                        const double B = 0.848013 + cosAngle * (-1.06021 + cosAngle * 0.215638);
                        t = t + t * t2 * (t - 1) * (A * t2 * t2 + B);
                    }
                    auto approxFractionalNormal = util::unit(*prevNormal * (1.0 - t) + *nextNormal * t);
                    addPieSliceVertex(*currentCoordinate,
                                      distance,
                                      approxFractionalNormal,
                                      lineTurnsLeft,
                                      startVertex,
                                      triangleStore,
                                      options.clipDistances);
                }
            }

            // Start next segment
            if (nextCoordinate) {
                addCurrentVertex(*currentCoordinate,
                                 distance,
                                 *nextNormal,
                                 -offsetA,
                                 -offsetB,
                                 false,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);
            }

        } else if (!middleVertex && currentCap == style::LineCapType::Butt) {
            if (!startOfLine) {
                // Close previous segment with a butt
                addCurrentVertex(*currentCoordinate,
                                 distance,
                                 *prevNormal,
                                 0,
                                 0,
                                 false,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);
            }

            // Start next segment with a butt
            if (nextCoordinate) {
                addCurrentVertex(*currentCoordinate,
                                 distance,
                                 *nextNormal,
                                 0,
                                 0,
                                 false,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);
            }

        } else if (!middleVertex && currentCap == style::LineCapType::Square) {
            if (!startOfLine) {
                // Close previous segment with a square cap
                addCurrentVertex(*currentCoordinate,
                                 distance,
                                 *prevNormal,
                                 1,
                                 1,
                                 false,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);

                // The segment is done. Unset vertices to disconnect segments.
                e1 = e2 = -1;
            }

            // Start next segment
            if (nextCoordinate) {
                addCurrentVertex(*currentCoordinate,
                                 distance,
                                 *nextNormal,
                                 -1,
                                 -1,
                                 false,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);
            }

        } else if (middleVertex ? currentJoin == style::LineJoinType::Round : currentCap == style::LineCapType::Round) {
            if (!startOfLine) {
                // Close previous segment with a butt
                addCurrentVertex(*currentCoordinate,
                                 distance,
                                 *prevNormal,
                                 0,
                                 0,
                                 false,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);

                // Add round cap or linejoin at end of segment
                addCurrentVertex(*currentCoordinate,
                                 distance,
                                 *prevNormal,
                                 1,
                                 1,
                                 true,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);

                // The segment is done. Unset vertices to disconnect segments.
                e1 = e2 = -1;
            }

            // Start next segment with a butt
            if (nextCoordinate) {
                // Add round cap before first segment
                addCurrentVertex(*currentCoordinate,
                                 distance,
                                 *nextNormal,
                                 -1,
                                 -1,
                                 true,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);

                addCurrentVertex(*currentCoordinate,
                                 distance,
                                 *nextNormal,
                                 0,
                                 0,
                                 false,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);
            }
        }

        if (isSharpCorner && i < len - 1) {
            const auto nextSegmentLength = util::dist<double>(*currentCoordinate, *nextCoordinate);
            if (nextSegmentLength > 2 * sharpCornerOffset) {
                GeometryCoordinate newCurrentVertex = *currentCoordinate +
                                                      convertPoint<int16_t>(util::round(
                                                          convertPoint<double>(*nextCoordinate - *currentCoordinate) *
                                                          (sharpCornerOffset / nextSegmentLength)));
                distance += util::dist<double>(newCurrentVertex, *currentCoordinate);
                addCurrentVertex(newCurrentVertex,
                                 distance,
                                 *nextNormal,
                                 0,
                                 0,
                                 false,
                                 startVertex,
                                 triangleStore,
                                 options.clipDistances);
                currentCoordinate = newCurrentVertex;
            }
        }

        startOfLine = false;
    }

    // add segment(s) and indices
    const std::size_t endVertex = vertices.elements();
    const std::size_t vertexCount = endVertex - startVertex;

    if (segments.empty() ||
        getSegment(segments.back()).vertexLength + vertexCount > std::numeric_limits<uint16_t>::max()) {
        segments.emplace_back(createSegment(startVertex, indexes.elements()));
    }

    auto& segment = getSegment(segments.back());
    assert(segment.vertexLength <= std::numeric_limits<uint16_t>::max());
    const uint16_t index = static_cast<uint16_t>(segment.vertexLength);

    if (indexes.empty()) {
        indexes.reserve(triangleStore.size() * 3);
    }
    for (const auto& triangle : triangleStore) {
        indexes.emplace_back(index + triangle.a, index + triangle.b, index + triangle.c);
    }

    segment.vertexLength += vertexCount;
    segment.indexLength += triangleStore.size() * 3;
}

template <class PLV, class PS>
void PolylineGenerator<PLV, PS>::addCurrentVertex(const GeometryCoordinate& currentCoordinate,
                                                  double& distance,
                                                  const Point<double>& normal,
                                                  double endLeft,
                                                  double endRight,
                                                  bool round,
                                                  std::size_t startVertex,
                                                  std::vector<TriangleElement>& triangleStore,
                                                  std::optional<PolylineGeneratorDistances> lineDistances) {
    Point<double> extrude = normal;
    const double scaledDistance = lineDistances ? lineDistances->scaleToMaxLineDistance(distance) : distance;

    if (endLeft) extrude = extrude - (util::perp(normal) * endLeft);
    vertices.emplace_back(layoutVertex(currentCoordinate,
                                       extrude,
                                       round,
                                       false,
                                       static_cast<int8_t>(endLeft),
                                       static_cast<int32_t>(scaledDistance * LINE_DISTANCE_SCALE)));
    e3 = vertices.elements() - 1 - startVertex;
    if (e1 >= 0 && e2 >= 0) {
        triangleStore.emplace_back(static_cast<uint16_t>(e1), static_cast<uint16_t>(e2), static_cast<uint16_t>(e3));
    }
    e1 = e2;
    e2 = e3;

    extrude = normal * -1.0;
    if (endRight) extrude = extrude - (util::perp(normal) * endRight);
    vertices.emplace_back(layoutVertex(currentCoordinate,
                                       extrude,
                                       round,
                                       true,
                                       static_cast<int8_t>(-endRight),
                                       static_cast<int32_t>(scaledDistance * LINE_DISTANCE_SCALE)));
    e3 = vertices.elements() - 1 - startVertex;
    if (e1 >= 0 && e2 >= 0) {
        triangleStore.emplace_back(static_cast<uint16_t>(e1), static_cast<uint16_t>(e2), static_cast<uint16_t>(e3));
    }
    e1 = e2;
    e2 = e3;

    // There is a maximum "distance along the line" that we can store in the
    // buffers. When we get close to the distance, reset it to zero and add the
    // vertex again with a distance of zero. The max distance is determined by
    // the number of bits we allocate to `linesofar`.
    if (distance > MAX_LINE_DISTANCE / 2.0f && !lineDistances) {
        distance = 0.0;
        addCurrentVertex(
            currentCoordinate, distance, normal, endLeft, endRight, round, startVertex, triangleStore, lineDistances);
    }
}

template <class PLV, class PS>
void PolylineGenerator<PLV, PS>::addPieSliceVertex(const GeometryCoordinate& currentVertex,
                                                   double distance,
                                                   const Point<double>& extrude,
                                                   bool lineTurnsLeft,
                                                   std::size_t startVertex,
                                                   std::vector<TriangleElement>& triangleStore,
                                                   std::optional<PolylineGeneratorDistances> lineDistances) {
    Point<double> flippedExtrude = extrude * (lineTurnsLeft ? -1.0 : 1.0);
    if (lineDistances) {
        distance = lineDistances->scaleToMaxLineDistance(distance);
    }

    vertices.emplace_back(layoutVertex(
        currentVertex, flippedExtrude, false, lineTurnsLeft, 0, static_cast<int32_t>(distance * LINE_DISTANCE_SCALE)));
    e3 = vertices.elements() - 1 - startVertex;
    if (e1 >= 0 && e2 >= 0) {
        triangleStore.emplace_back(static_cast<uint16_t>(e1), static_cast<uint16_t>(e2), static_cast<uint16_t>(e3));
    }

    if (lineTurnsLeft) {
        e2 = e3;
    } else {
        e1 = e3;
    }
}

template class PolylineGenerator<gfx::DrawableBuilder::Impl::LineLayoutVertex,
                                 std::unique_ptr<gfx::Drawable::DrawSegment>>;

template class PolylineGenerator<LineLayoutVertex, SegmentBase>;

} // namespace gfx
} // namespace mbgl
