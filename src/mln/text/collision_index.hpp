#pragma once

#include <mln/geometry/feature_index.hpp>

#include <mln/text/collision_feature.hpp>
#include <mln/util/grid_index.hpp>
#include <mln/map/tile_projector.hpp>
#include <mln/map/transform_state.hpp>

#include <array>

namespace mln {

class PlacedSymbol;
class LabelPlaneProjector;

struct TileDistance;

struct PlacedFeatureResult {
    bool placed = false;
    bool offscreen = false;
    /// Behind the planet's horizon: hidden even when overlap is allowed.
    bool occluded = false;
};

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
                                        const CollisionBoundaries& tileEdges) const;
    PlacedFeatureResult placeFeature(
        const CollisionFeature& feature,
        Point<float> shift,
        Point<float> translation,
        const TileProjector& tileProjector,
        const LabelPlaneProjector& labelPlane,
        float textPixelRatio,
        const PlacedSymbol& symbol,
        float scale,
        float fontSize,
        bool allowOverlap,
        bool pitchWithMap,
        bool rotateWithMap,
        bool collisionDebug,
        const std::optional<CollisionBoundaries>& avoidEdges,
        const std::optional<std::function<bool(const RefIndexedSubfeature&)>>& collisionGroupPredicate,
        std::vector<ProjectedCollisionBox>& /*out*/
    );

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

    PlacedFeatureResult placeLineFeature(
        const CollisionFeature& feature,
        const TileProjector& tileProjector,
        const LabelPlaneProjector& labelPlane,
        float textPixelRatio,
        const PlacedSymbol& symbol,
        float scale,
        float fontSize,
        bool allowOverlap,
        bool pitchWithMap,
        bool collisionDebug,
        const std::optional<CollisionBoundaries>& avoidEdges,
        const std::optional<std::function<bool(const RefIndexedSubfeature&)>>& collisionGroupPredicate,
        std::vector<ProjectedCollisionBox>& /*out*/
    );

    float approximateTileDistance(const TileDistance& tileDistance,
                                  float lastSegmentAngle,
                                  float pixelsToTileUnits,
                                  float cameraToAnchorDistance,
                                  bool pitchWithMap);

    struct ProjectedAnchor {
        Point<float> point;
        float perspectiveRatio;
        float signedDistanceFromCamera;
        bool occluded;
    };

    ProjectedAnchor toViewport(const ProjectedTilePoint&) const;
    ProjectedAnchor projectAndGetPerspectiveRatio(const TileProjector&, const Point<float>& point) const;
    Point<float> projectPoint(const mat4& posMatrix, const Point<float>& point) const;
    Point<float> projectPoint(const TileProjector&, const Point<float>& point) const;
    CollisionBoundaries getProjectedCollisionBoundaries(const mat4& posMatrix,
                                                        Point<float> shift,
                                                        float textPixelRatio,
                                                        const CollisionBox& box) const;
    CollisionBoundaries getProjectedCollisionBoundaries(const ProjectedAnchor&,
                                                        Point<float> shift,
                                                        float textPixelRatio,
                                                        const CollisionBox& box) const;

    struct ProjectedBox {
        CollisionBoundaries boundaries;
        bool allPointsOccluded;
    };
    /// Pitched or rotated boxes on the globe: the box outline is sampled at eight points and projected,
    /// so it foreshortens toward the horizon the way the label does.
    ProjectedBox projectCollisionBox(const CollisionBox& box,
                                     float tileToViewport,
                                     float scale,
                                     const TileProjector&,
                                     bool pitchWithMap,
                                     bool rotateWithMap,
                                     const ProjectedAnchor& projectedPoint,
                                     Point<float> shift,
                                     Point<float> translation) const;

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
