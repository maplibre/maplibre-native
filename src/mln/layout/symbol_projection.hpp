#pragma once

#include <mln/util/mat4.hpp>
#include <mln/map/tile_projector.hpp>
#include <mln/gfx/vertex_buffer.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>

#include <optional>

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
void getTileSkewVectors(const TransformState& state, vec2& vecEast, vec2& vecSouth);

/// Pitched labels: tile units to the pitched map plane in pixels. Viewport labels: clip space to viewport pixels.
mat4 getLabelPlaneMatrix(bool pitchWithMap, bool rotateWithMap, const TransformState& state, float pixelsToTileUnits);
/// The inverse for the shader: pitched labels back to tile units, viewport labels to clip space.
mat4 getGlCoordMatrix(bool pitchWithMap, bool rotateWithMap, const TransformState& state, float pixelsToTileUnits);

using PointAndCameraDistance = std::pair<Point<float>, float>;
PointAndCameraDistance project(const Point<float>& point, const mat4& matrix);

/// Projects tile points to the label plane a symbol is laid out in, through the tile's projection.
class LabelPlaneProjector {
public:
    LabelPlaneProjector(const TileProjector&,
                        bool pitchWithMap,
                        bool rotateWithMap,
                        float pixelsToTileUnits,
                        Point<float> translation = {0, 0});

    /// Tile units to the label plane; distance and occlusion come from the projection.
    ProjectedTilePoint project(const Point<float>& tilePoint) const;
    /// Label plane to clip space, for orientation checks.
    Point<float> toClipSpace(const Point<float>& labelPlanePoint) const;
    /// Tile units straight to clip space.
    ProjectedTilePoint toClipSpaceFromTile(const Point<float>& tilePoint) const;

private:
    TileProjector tile;
    bool pitchWithMap;
    Point<float> translation;
    mat4 pitchedLabelPlaneMatrix;
    mat4 pitchedLabelPlaneMatrixInverse;
    float width;
    float height;
};

/// The projections of one symbol's line vertices, so each vertex is projected once however many glyphs walk it.
class LineProjectionCache {
public:
    void reset(std::size_t vertexCount) {
        points.assign(vertexCount, std::nullopt);
        occluded = false;
    }
    const ProjectedTilePoint& get(std::size_t index, const GeometryCoordinates& line, const LabelPlaneProjector&);
    /// Whether any vertex projected so far is behind the globe's horizon.
    bool anyOccluded() const { return occluded; }

private:
    std::vector<std::optional<ProjectedTilePoint>> points;
    bool occluded = false;
};

void reprojectLineLabels(SymbolBucket::DynamicAttributeVector&,
                         const std::vector<PlacedSymbol>&,
                         const TileProjector&,
                         bool pitchWithMap,
                         bool rotateWithMap,
                         bool keepUpright,
                         const RenderTile&,
                         const SymbolSizeBinder& sizeBinder,
                         const TransformState&);

std::optional<std::pair<PlacedGlyph, PlacedGlyph>> placeFirstAndLastGlyph(float fontScale,
                                                                          float lineOffsetX,
                                                                          float lineOffsetY,
                                                                          bool flip,
                                                                          const Point<float>& anchorPoint,
                                                                          const Point<float>& tileAnchorPoint,
                                                                          const PlacedSymbol& symbol,
                                                                          const LabelPlaneProjector&,
                                                                          LineProjectionCache&,
                                                                          bool returnTileDistance);

void hideGlyphs(std::size_t numGlyphs, SymbolBucket::DynamicAttributeVector& dynamicVertexArray);
void addDynamicAttributes(const Point<float>& anchorPoint,
                          float angle,
                          SymbolBucket::DynamicAttributeVector& dynamicAttributeData);

} // end namespace mln
