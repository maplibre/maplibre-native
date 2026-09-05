#pragma once

#include <mln/util/mat4.hpp>
#include <mln/gfx/vertex_buffer.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>

#include <functional>

namespace mln {

class TransformState;
class RenderTile;
class SymbolSizeBinder;
class PlacedSymbol;
namespace style {
class SymbolPropertyValues;
} // end namespace style

struct TileDistance {
    TileDistance(float prevTileDistance_, float lastSegmentViewportDistance_)
        : prevTileDistance(prevTileDistance_),
          lastSegmentViewportDistance(lastSegmentViewportDistance_) {}
    float prevTileDistance;
    float lastSegmentViewportDistance;
};

struct PlacedGlyph {
    PlacedGlyph() = default;

    PlacedGlyph(Point<float> point_, float angle_, std::optional<TileDistance> tileDistance_)
        : point(point_),
          angle(angle_),
          tileDistance(std::move(tileDistance_)) {}
    PlacedGlyph(PlacedGlyph&& other) noexcept
        : point(other.point),
          angle(other.angle),
          tileDistance(std::move(other.tileDistance)) {}
    PlacedGlyph(const PlacedGlyph& other) = default;
    Point<float> point;
    float angle;
    std::optional<TileDistance> tileDistance;
};

float evaluateSizeForFeature(const ZoomEvaluatedSize& zoomEvaluatedSize, const PlacedSymbol& placedSymbol);
mat4 getLabelPlaneMatrix(
    const mat4& posMatrix, bool pitchWithMap, bool rotateWithMap, const TransformState& state, float pixelsToTileUnits);
mat4 getGlCoordMatrix(
    const mat4& posMatrix, bool pitchWithMap, bool rotateWithMap, const TransformState& state, float pixelsToTileUnits);

using PointAndCameraDistance = std::pair<Point<float>, float>;
/// Tile-local point (0..EXTENT) -> terrain elevation in metres (exaggeration applied). With 3D
/// terrain, line-placed labels are projected on the CPU, so their glyph anchors must be lifted
/// here the way maplibre-gl-js's `getElevation` callback does; nullptr = flat.
using SymbolElevationFn = std::function<float(const Point<float>&)>;
PointAndCameraDistance project(const Point<float>& point,
                               const mat4& matrix,
                               const SymbolElevationFn* getElevation = nullptr);

void reprojectLineLabels(SymbolBucket::DynamicAttributeVector&,
                         const std::vector<PlacedSymbol>&,
                         const mat4& posMatrix,
                         bool pitchWithMap,
                         bool rotateWithMap,
                         bool keepUpright,
                         const RenderTile&,
                         const SymbolSizeBinder& sizeBinder,
                         const TransformState&,
                         const SymbolElevationFn* getElevation = nullptr);

std::optional<std::pair<PlacedGlyph, PlacedGlyph>> placeFirstAndLastGlyph(
    float fontScale,
    float lineOffsetX,
    float lineOffsetY,
    bool flip,
    const Point<float>& anchorPoint,
    const Point<float>& tileAnchorPoint,
    const PlacedSymbol& symbol,
    const mat4& labelPlaneMatrix,
    bool returnTileDistance,
    const SymbolElevationFn* getElevation = nullptr);

void hideGlyphs(std::size_t numGlyphs, SymbolBucket::DynamicAttributeVector& dynamicVertexArray);
void addDynamicAttributes(const Point<float>& anchorPoint,
                          float angle,
                          SymbolBucket::DynamicAttributeVector& dynamicAttributeData);

} // end namespace mln
