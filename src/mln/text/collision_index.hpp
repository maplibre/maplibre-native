#pragma once

#include <mln/geometry/feature_index.hpp>

#include <mln/text/collision_feature.hpp>
#include <mln/util/grid_index.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/util/geometry.hpp>

#include <functional>

#include <array>

namespace mln {

/// Same alias as in layout/symbol_projection.hpp (not included here: its `project`
/// overloads would hide unrelated `project` calls in translation units using this header).
using SymbolElevationFn = std::function<float(const Point<float>&)>;

class PlacedSymbol;

struct TileDistance;

using CollisionBoundaries = std::array<float, 4>; // [x1, y1, x2, y2]
struct IntersectStatus {
    enum Flags : uint8_t {
        None = 0,
        HorizontalBorders = 1 << 0,
        VerticalBorders = 1 << 1
    };
    Flags flags = None;
    // Assuming tile border divides box in two sections
    int minSectionLength = 0;
};
class CollisionIndex {
public:
    using CollisionGrid = GridIndex<IndexedSubfeature>;

    explicit CollisionIndex(const TransformState&, MapMode);
    IntersectStatus intersectsTileEdges(const CollisionBox&,
                                        Point<float> shift,
                                        const mat4& posMatrix,
                                        float textPixelRatio,
                                        const CollisionBoundaries& tileEdges,
                                        const SymbolElevationFn* getElevation = nullptr) const;
    std::pair<bool, bool> placeFeature(
        const CollisionFeature& feature,
        Point<float> shift,
        const mat4& posMatrix,
        const mat4& labelPlaneMatrix,
        float textPixelRatio,
        const PlacedSymbol& symbol,
        float scale,
        float fontSize,
        bool allowOverlap,
        bool pitchWithMap,
        bool collisionDebug,
        const std::optional<CollisionBoundaries>& avoidEdges,
        const std::optional<std::function<bool(const RefIndexedSubfeature&)>>& collisionGroupPredicate,
        std::vector<ProjectedCollisionBox>& /*out*/,
        /// On 3D terrain: tile-local point -> exaggerated metres, so the boxes are
        /// projected where the elevated symbol actually renders (gl-js getElevation)
        const SymbolElevationFn* getElevation = nullptr);

    void insertFeature(const CollisionFeature& feature,
                       const std::vector<ProjectedCollisionBox>&,
                       bool ignorePlacement,
                       uint32_t bucketInstanceId,
                       uint16_t collisionGroupId);

    std::unordered_map<uint32_t, std::vector<IndexedSubfeature>> queryRenderedSymbols(const ScreenLineString&) const;

    CollisionBoundaries projectTileBoundaries(const mat4& posMatrix) const;

    const TransformState& getTransformState() const { return transformState; }

    float getViewportPadding() const { return viewportPadding; }

private:
    bool isOffscreen(const CollisionBoundaries&) const;
    bool isInsideGrid(const CollisionBoundaries&) const;
    bool isInsideTile(const CollisionBoundaries& boundaries, const CollisionBoundaries& tileBoundaries) const;
    bool overlapsTile(const CollisionBoundaries& boundaries, const CollisionBoundaries& tileBoundaries) const;

    std::pair<bool, bool> placeLineFeature(
        const CollisionFeature& feature,
        const mat4& posMatrix,
        const mat4& labelPlaneMatrix,
        float textPixelRatio,
        const PlacedSymbol& symbol,
        float scale,
        float fontSize,
        bool allowOverlap,
        bool pitchWithMap,
        bool collisionDebug,
        const std::optional<CollisionBoundaries>& avoidEdges,
        const std::optional<std::function<bool(const RefIndexedSubfeature&)>>& collisionGroupPredicate,
        std::vector<ProjectedCollisionBox>& /*out*/,
        const SymbolElevationFn* getElevation);

    float approximateTileDistance(const TileDistance& tileDistance,
                                  float lastSegmentAngle,
                                  float pixelsToTileUnits,
                                  float cameraToAnchorDistance,
                                  bool pitchWithMap);

    std::pair<float, float> projectAnchor(const mat4& posMatrix,
                                          const Point<float>& point,
                                          const SymbolElevationFn* getElevation) const;
    std::pair<Point<float>, float> projectAndGetPerspectiveRatio(const mat4& posMatrix,
                                                                 const Point<float>& point,
                                                                 const SymbolElevationFn* getElevation) const;
    Point<float> projectPoint(const mat4& posMatrix,
                              const Point<float>& point,
                              const SymbolElevationFn* getElevation = nullptr) const;
    CollisionBoundaries getProjectedCollisionBoundaries(const mat4& posMatrix,
                                                        Point<float> shift,
                                                        float textPixelRatio,
                                                        const CollisionBox& box,
                                                        const SymbolElevationFn* getElevation) const;

    const TransformState transformState;

    const float viewportPadding;
    CollisionGrid collisionGrid;
    CollisionGrid ignoredGrid;

    const float screenRightBoundary;
    const float screenBottomBoundary;
    const float gridRightBoundary;
    const float gridBottomBoundary;

    const float pitchFactor;
};

} // namespace mln
