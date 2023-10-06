#pragma once

#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/util/math.hpp>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
#include <functional>
#include <optional>

namespace mbgl {
namespace util {

namespace {
// The number of bits that is used to store the line distance in the buffer.
constexpr int LINE_DISTANCE_BUFFER_BITS = 14;

// We don't have enough bits for the line distance as we'd like to have, so
// use this value to scale the line distance (in tile units) down to a smaller
// value. This lets us store longer distances while sacrificing precision.
constexpr float LINE_DISTANCE_SCALE = 1.0 / 2.0;

// The maximum line distance, in tile units, that fits in the buffer.
constexpr auto MAX_LINE_DISTANCE = static_cast<float>((1u << LINE_DISTANCE_BUFFER_BITS) / LINE_DISTANCE_SCALE);

} // namespace

class PolylineGeneratorDistances {
public:
    PolylineGeneratorDistances(double clipStart_, double clipEnd_, double total_)
        : clipStart(clipStart_),
          clipEnd(clipEnd_),
          total(total_) {}

    // Scale line distance from tile units to [0, 2^15).
    double scaleToMaxLineDistance(double tileDistance) const {
        double relativeTileDistance = tileDistance / total;
        if (std::isinf(relativeTileDistance) || std::isnan(relativeTileDistance)) {
            assert(false);
            relativeTileDistance = 0.0;
        }
        return (relativeTileDistance * (clipEnd - clipStart) + clipStart) * (MAX_LINE_DISTANCE - 1);
    }

private:
    double clipStart;
    double clipEnd;
    double total;
};

struct PolylineGeneratorOptions {
    FeatureType type{FeatureType::LineString};
    style::LineJoinType joinType{style::LineJoinType::Miter};
    float miterLimit{2.f};
    style::LineCapType beginCap{style::LineCapType::Butt};
    style::LineCapType endCap{style::LineCapType::Butt};
    float roundLimit{1.f};
    uint32_t overscaling{1}; // TODO: what is this???
    std::optional<PolylineGeneratorDistances> lineDistances;
};

template <class PolylineLayoutVertex, class PolylineSegment>
class PolylineGenerator {
public:
    using Vertices = gfx::VertexVector<PolylineLayoutVertex>;
    using Segments = std::vector<PolylineSegment>;
    using LayoutVertexFunc = std::function<PolylineLayoutVertex(
        Point<int16_t> p, Point<double> e, bool round, bool up, int8_t dir, int32_t linesofar /*= 0*/)>;
    using CreateSegmentFunc = std::function<PolylineSegment(std::size_t vertexOffset, std::size_t indexOffset)>;
    using GetSegmentFunc = std::function<mbgl::SegmentBase&(PolylineSegment& segment)>;
    using Indexes = gfx::IndexVector<gfx::Triangles>;

public:
    PolylineGenerator(Vertices& polylineVertices,
                      LayoutVertexFunc layoutVertexFunc,
                      Segments& polylineSegments,
                      CreateSegmentFunc createSegmentFunc,
                      GetSegmentFunc getSegmentFunc,
                      Indexes& polylineIndexes);
    ~PolylineGenerator() = default;

    void generate(const GeometryCoordinates& coordinates, const PolylineGeneratorOptions& options);

private:
    struct TriangleElement;

    void addCurrentVertex(const GeometryCoordinate& currentCoordinate,
                          double& distance,
                          const Point<double>& normal,
                          double endLeft,
                          double endRight,
                          bool round,
                          std::size_t startVertex,
                          std::vector<TriangleElement>& triangleStore,
                          std::optional<PolylineGeneratorDistances> lineDistances);
    void addPieSliceVertex(const GeometryCoordinate& currentVertex,
                           double distance,
                           const Point<double>& extrude,
                           bool lineTurnsLeft,
                           std::size_t startVertex,
                           std::vector<TriangleElement>& triangleStore,
                           std::optional<PolylineGeneratorDistances> lineDistances);

private:
    Vertices& vertices;
    LayoutVertexFunc layoutVertex;
    Segments& segments;
    CreateSegmentFunc createSegment;
    GetSegmentFunc getSegment;
    Indexes& indexes;

    std::ptrdiff_t e1;
    std::ptrdiff_t e2;
    std::ptrdiff_t e3;
};

} // namespace util
} // namespace mbgl