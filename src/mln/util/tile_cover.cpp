#include <array>
#include <span>
#include <mln/math/angles.hpp>
#include <mln/math/log2.hpp>
#include <mln/map/vertical_perspective_projection.hpp>
#include <mln/util/bounding_volumes.hpp>
#include <mln/util/mat3.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/interpolate.hpp>
#include <mln/util/tile_coordinate.hpp>
#include <mln/util/tile_cover.hpp>
#include <mln/util/tile_cover_impl.hpp>

#include <cassert>
#include <functional>
#include <limits>
#include <list>

using namespace std::numbers;

namespace mln {

namespace {

using ScanLine = const std::function<void(int32_t x0, int32_t x1, int32_t y)>;

// Taken from polymaps src/Layer.js
// https://github.com/simplegeo/polymaps/blob/master/src/Layer.js#L333-L383
struct edge {
    double x0 = 0, y0 = 0;
    double x1 = 0, y1 = 0;
    double dx = 0, dy = 0;

    edge(Point<double> a, Point<double> b) {
        if (a.y > b.y) std::swap(a, b);
        x0 = a.x;
        y0 = a.y;
        x1 = b.x;
        y1 = b.y;
        dx = b.x - a.x;
        dy = b.y - a.y;
    }
};

// scan-line conversion
void scanSpans(edge e0, edge e1, int32_t ymin, int32_t ymax, ScanLine& scanLine) {
    const double y0 = ::fmax(ymin, std::floor(e1.y0));
    const double y1 = ::fmin(ymax, std::ceil(e1.y1));

    // sort edges by x-coordinate
    if ((e0.x0 == e1.x0 && e0.y0 == e1.y0) ? (e0.x0 + e1.dy / e0.dy * e0.dx < e1.x1)
                                           : (e0.x1 - e1.dy / e0.dy * e0.dx < e1.x0)) {
        std::swap(e0, e1);
    }

    // scan lines!
    const double m0 = e0.dx / e0.dy;
    const double m1 = e1.dx / e1.dy;
    const double d0 = e0.dx > 0;       // use y + 1 to compute x0
    const double d1 = e1.dx < 0;       // use y + 1 to compute x1
    for (double y = y0; y < y1; y++) { // NOLINT(clang-analyzer-security.FloatLoopCounter)
        double x0 = m0 * ::fmax(0, ::fmin(e0.dy, y + d0 - e0.y0)) + e0.x0;
        double x1 = m1 * ::fmax(0, ::fmin(e1.dy, y + d1 - e1.y0)) + e1.x0;
        scanLine(static_cast<int32_t>(std::floor(x1)), static_cast<int32_t>(std::ceil(x0)), static_cast<int32_t>(y));
    }
}

// scan-line conversion
void scanTriangle(const Point<double>& a,
                  const Point<double>& b,
                  const Point<double>& c,
                  int32_t ymin,
                  int32_t ymax,
                  ScanLine& scanLine) {
    edge ab = edge(a, b);
    edge bc = edge(b, c);
    edge ca = edge(c, a);

    // sort edges by y-length
    if (ab.dy > bc.dy) {
        std::swap(ab, bc);
    }
    if (ab.dy > ca.dy) {
        std::swap(ab, ca);
    }
    if (bc.dy > ca.dy) {
        std::swap(bc, ca);
    }

    // scan span! scan span!
    if (ab.dy) scanSpans(ca, ab, ymin, ymax, scanLine);
    if (bc.dy) scanSpans(ca, bc, ymin, ymax, scanLine);
}

} // namespace

namespace util {

namespace {

std::vector<UnwrappedTileID> tileCover(const Point<double>& tl,
                                       const Point<double>& tr,
                                       const Point<double>& br,
                                       const Point<double>& bl,
                                       const Point<double>& c,
                                       uint8_t z) {
    const int32_t tiles = 1 << z;

    struct ID {
        int32_t x, y;
        double sqDist;
    };

    std::vector<ID> t;

    // skip the first few allocations, assuming we usually end up with at least a few tiles
    t.reserve(8);

    auto scanLine = [&](int32_t x0, int32_t x1, int32_t y) {
        int32_t x;
        if (y >= 0 && y <= tiles) {
            for (x = x0; x < x1; ++x) {
                const auto dx = x + 0.5 - c.x;
                const auto dy = y + 0.5 - c.y;
                t.emplace_back(ID{.x = x, .y = y, .sqDist = dx * dx + dy * dy});
            }
        }
    };

    // Divide the screen up in two triangles and scan each of them:
    // \---+
    // | \ |
    // +---\.
    scanTriangle(tl, tr, br, 0, tiles, scanLine);
    scanTriangle(br, bl, tl, 0, tiles, scanLine);

    // Sort first by distance, then by x/y.
    std::sort(t.begin(), t.end(), [](const ID& a, const ID& b) noexcept {
        return std::tie(a.sqDist, a.x, a.y) < std::tie(b.sqDist, b.x, b.y);
    });

    // Erase duplicate tile IDs (they typically occur at the common side of both triangles).
    t.erase(std::unique(t.begin(), t.end(), [](const ID& a, const ID& b) { return a.x == b.x && a.y == b.y; }),
            t.end());

    std::vector<UnwrappedTileID> result;
    result.reserve(t.size());
    for (const auto& id : t) {
        result.emplace_back(z, id.x, id.y);
    }
    return result;
}

} // namespace

int32_t coveringZoomLevel(double zoom, style::SourceType type, uint16_t size) noexcept {
    zoom += util::log2(util::tileSize_D / size);
    if (type == style::SourceType::Raster || type == style::SourceType::Video) {
        return static_cast<int32_t>(std::round(zoom));
    } else {
        return static_cast<int32_t>(std::floor(zoom));
    }
}

// The globe tile cover: a port of GL JS `coveringTiles` with `GlobeCoveringTilesDetailsProvider`. Tiles are
// tested against the camera frustum in unit-sphere space with a convex bounding volume each, and against
// the horizon plane, so the far side of the planet loads nothing.
namespace {
namespace globe {

// A tile's bounding volume in unit-sphere space: a box at zoom 0 and 1, a wedge with up to eight corners after.
struct ConvexVolume {
    std::array<vec3, 8> points{};
    std::size_t pointCount = 0;
    std::array<vec4, 6> planes{};

    void addPoint(const vec3& point) {
        assert(pointCount < points.size());
        points[pointCount++] = point;
    }
};

IntersectionResult intersectsFrustum(const ConvexVolume& volume, const Frustum& frustum) {
    bool fullyInside = true;
    for (const vec4& plane : frustum.getPlanes()) {
        std::size_t passed = 0;
        for (std::size_t i = 0; i < volume.pointCount; ++i) {
            const vec3& point = volume.points[i];
            if (plane[0] * point[0] + plane[1] * point[1] + plane[2] * point[2] + plane[3] >= 0) {
                passed++;
            }
        }
        if (passed == 0) {
            return IntersectionResult::Separate;
        }
        if (passed < volume.pointCount) {
            fullyInside = false;
        }
    }
    if (fullyInside) {
        return IntersectionResult::Contains;
    }
    for (const vec4& plane : volume.planes) {
        std::size_t passed = 0;
        for (const vec3& point : frustum.getPoints()) {
            if (plane[0] * point[0] + plane[1] * point[1] + plane[2] * point[2] + plane[3] >= 0) {
                passed++;
            }
        }
        if (passed == 0) {
            return IntersectionResult::Separate;
        }
    }
    return IntersectionResult::Intersects;
}

IntersectionResult intersectsPlane(const ConvexVolume& volume, const vec4& plane) {
    std::size_t positive = 0;
    for (std::size_t i = 0; i < volume.pointCount; ++i) {
        const vec3& point = volume.points[i];
        if (plane[0] * point[0] + plane[1] * point[1] + plane[2] * point[2] + plane[3] >= 0) {
            positive++;
        }
    }
    if (positive == volume.pointCount) {
        return IntersectionResult::Contains;
    }
    if (positive == 0) {
        return IntersectionResult::Separate;
    }
    return IntersectionResult::Intersects;
}

IntersectionResult isTileVisible(const Frustum& frustum, const ConvexVolume& volume, const vec4& plane) {
    const auto frustumTest = intersectsFrustum(volume, frustum);
    if (frustumTest == IntersectionResult::Separate) {
        return frustumTest;
    }
    const auto planeTest = intersectsPlane(volume, plane);
    if (planeTest == IntersectionResult::Separate) {
        return IntersectionResult::Separate;
    }
    if (frustumTest == IntersectionResult::Contains && planeTest == IntersectionResult::Contains) {
        return IntersectionResult::Contains;
    }
    return IntersectionResult::Intersects;
}

vec3 threePlaneIntersection(const vec4& p0, const vec4& p1, const vec4& p2) {
    const vec3 n0 = {{p0[0], p0[1], p0[2]}};
    const vec3 n1 = {{p1[0], p1[1], p1[2]}};
    const vec3 n2 = {{p2[0], p2[1], p2[2]}};
    const double det = vec3Dot(n0, vec3Cross(n1, n2));
    if (det == 0) {
        return {{0, 0, 0}};
    }
    vec3 sum = vec3Scale(vec3Cross(n1, n2), -p0[3]);
    sum = vec3Sub(sum, vec3Scale(vec3Cross(n2, n0), p1[3]));
    sum = vec3Sub(sum, vec3Scale(vec3Cross(n0, n1), p2[3]));
    return vec3Scale(sum, 1.0 / det);
}

ConvexVolume aabbVolume(const vec3& min, const vec3& max) {
    ConvexVolume volume;
    for (int i = 0; i < 8; i++) {
        volume.addPoint({{(i & 1) ? max[0] : min[0], (i & 2) ? max[1] : min[1], (i & 4) ? max[2] : min[2]}});
    }
    volume.planes = {{{{-1, 0, 0, max[0]}},
                      {{1, 0, 0, -min[0]}},
                      {{0, -1, 0, max[1]}},
                      {{0, 1, 0, -min[1]}},
                      {{0, 0, -1, max[2]}},
                      {{0, 0, 1, -min[2]}}}};
    return volume;
}

std::pair<double, double> axisMinMax(const vec3& axis, std::span<const vec3> points) {
    double min = std::numeric_limits<double>::infinity();
    double max = -std::numeric_limits<double>::infinity();
    for (const vec3& point : points) {
        const double dot = vec3Dot(axis, point);
        min = std::min(min, dot);
        max = std::max(max, dot);
    }
    return {min, max};
}

vec3 toSphere(double x, double y, const CanonicalTileID& tile) {
    return VerticalPerspectiveProjection::tileCoordinatesToSphere({x, y}, UnwrappedTileID(0, tile));
}

constexpr double maxMercatorHorizonAngle = 89.25;
constexpr double assumedMaxFeatureHeightMeters = 500;
constexpr double tileCullingHorizonOnsetDegrees = 15;

// Near the bottom of a steeply pitched view, tiles are culled with room for the tallest feature they might hold,
// so buildings do not vanish before their tile leaves the screen (GL JS `getElevationForTileCulling`).
double elevationForTileCulling(double pitchDegrees, double fovDegrees) {
    const double bottomEdgeDegreesAboveHorizontal = maxMercatorHorizonAngle - pitchDegrees - fovDegrees / 2;
    const double proximityToHorizon = std::clamp(
        (tileCullingHorizonOnsetDegrees - bottomEdgeDegreesAboveHorizontal) / tileCullingHorizonOnsetDegrees, 0.0, 1.0);
    return proximityToHorizon * assumedMaxFeatureHeightMeters;
}

// `elevation` in meters grows the volume above the sphere's surface.
ConvexVolume tileBoundingVolume(const CanonicalTileID& tile, double elevation) {
    // Distances from the planet's center in a world where the surface is at 1.
    const double minRadius = std::min(0.0, elevation) / util::EARTH_RADIUS_M + 1.0;
    const double maxRadius = std::max(0.0, elevation) / util::EARTH_RADIUS_M + 1.0;
    if (tile.z == 0) {
        return aabbVolume({{-maxRadius, -maxRadius, -maxRadius}}, {{maxRadius, maxRadius, maxRadius}});
    }
    if (tile.z == 1) {
        // X is 1 at lng 90E, Y is 1 at the north pole, Z is 1 at null island.
        return aabbVolume({{tile.x == 0 ? -maxRadius : 0.0, tile.y == 0 ? 0.0 : -maxRadius, -maxRadius}},
                          {{tile.x == 0 ? 0.0 : maxRadius, tile.y == 0 ? maxRadius : 0.0, maxRadius}});
    }
    const std::array<vec3, 4> corners = {{toSphere(0, 0, tile),
                                          toSphere(util::EXTENT, 0, tile),
                                          toSphere(util::EXTENT, util::EXTENT, tile),
                                          toSphere(0, util::EXTENT, tile)}};
    // Corners at both radii, a pole if the tile touches one, the center, and the bulging edge midpoint.
    std::array<vec3, 11> extremes{};
    std::size_t extremeCount = 0;
    for (const vec3& corner : corners) {
        extremes[extremeCount++] = vec3Scale(corner, maxRadius);
    }
    if (maxRadius != minRadius) {
        for (const vec3& corner : corners) {
            extremes[extremeCount++] = vec3Scale(corner, minRadius);
        }
    }
    const uint32_t lastRow = (1u << tile.z) - 1;
    if (tile.y == 0) {
        extremes[extremeCount++] = {{0, 1, 0}};
    }
    if (tile.y == lastRow) {
        extremes[extremeCount++] = {{0, -1, 0}};
    }

    // The up/down axis is the tile center; north/south is orthogonal to it; the east and west planes follow
    // the tile's meridian edges and are not parallel, so the volume is a wedge rather than a box.
    const vec3 center = toSphere(util::EXTENT / 2.0, util::EXTENT / 2.0, tile);
    const vec3 centerEast = vec3Normalize(vec3Cross({{0, 1, 0}}, center));
    const vec3 north = vec3Normalize(vec3Cross(center, centerEast));
    const vec3 axisEast = vec3Normalize(vec3Cross(corners[2], corners[1]));
    const vec3 axisWest = vec3Normalize(vec3Cross(corners[0], corners[3]));

    extremes[extremeCount++] = vec3Scale(center, maxRadius);
    // The edge midpoint that bulges away from the center: the north edge in the south hemisphere, the south edge in the
    // north.
    if (tile.y >= (1u << tile.z) / 2) {
        extremes[extremeCount++] = vec3Scale(toSphere(util::EXTENT / 2.0, 0, tile), maxRadius);
    } else {
        extremes[extremeCount++] = vec3Scale(toSphere(util::EXTENT / 2.0, util::EXTENT, tile), maxRadius);
    }
    const std::span<const vec3> extremeSpan(extremes.data(), extremeCount);

    const auto [upMin, upMax] = axisMinMax(center, extremeSpan);
    const auto [northMin, northMax] = axisMinMax(north, extremeSpan);
    const vec4 planeUp = {{-center[0], -center[1], -center[2], upMax}};
    const vec4 planeDown = {{center[0], center[1], center[2], -upMin}};
    const vec4 planeNorth = {{-north[0], -north[1], -north[2], northMax}};
    const vec4 planeSouth = {{north[0], north[1], north[2], -northMin}};
    const vec4 planeEast = {{axisEast[0], axisEast[1], axisEast[2], 0}};
    const vec4 planeWest = {{axisWest[0], axisWest[1], axisWest[2], 0}};

    ConvexVolume volume;
    if (tile.y == 0) {
        volume.addPoint(threePlaneIntersection(planeWest, planeEast, planeUp));
        volume.addPoint(threePlaneIntersection(planeWest, planeEast, planeDown));
    } else {
        volume.addPoint(threePlaneIntersection(planeNorth, planeEast, planeUp));
        volume.addPoint(threePlaneIntersection(planeNorth, planeEast, planeDown));
        volume.addPoint(threePlaneIntersection(planeNorth, planeWest, planeUp));
        volume.addPoint(threePlaneIntersection(planeNorth, planeWest, planeDown));
    }
    if (tile.y == lastRow) {
        volume.addPoint(threePlaneIntersection(planeWest, planeEast, planeUp));
        volume.addPoint(threePlaneIntersection(planeWest, planeEast, planeDown));
    } else {
        volume.addPoint(threePlaneIntersection(planeSouth, planeEast, planeUp));
        volume.addPoint(threePlaneIntersection(planeSouth, planeEast, planeDown));
        volume.addPoint(threePlaneIntersection(planeSouth, planeWest, planeUp));
        volume.addPoint(threePlaneIntersection(planeSouth, planeWest, planeDown));
    }
    volume.planes = {planeUp, planeDown, planeNorth, planeSouth, planeEast, planeWest};
    return volume;
}

// Distances below are in a world of size 1, to the nearer tile edge.
double distanceToTileSimple(double point, double tile, double tileSize) {
    const double delta = point - tile;
    return delta < 0 ? -delta : std::max(0.0, delta - tileSize);
}

double distanceToTileWrapX(double pointX, double pointY, double cornerX, double cornerY, double tileSize) {
    const double toPointX = pointX - cornerX;
    double distanceX;
    if (toPointX < 0) {
        distanceX = std::min(-toPointX, 1.0 + toPointX - tileSize);
    } else if (toPointX > tileSize) {
        distanceX = std::min(std::max(toPointX - tileSize, 0.0), 1.0 - toPointX);
    } else {
        distanceX = 0;
    }
    return std::max(distanceX, distanceToTileSimple(pointY, cornerY, tileSize));
}

// X wraps at the antimeridian; across a pole Y mirrors and X shifts by half the world.
double distanceToTile2d(double pointX, double pointY, const CanonicalTileID& tile) {
    const double scale = static_cast<double>(1u << tile.z);
    const double tileSize = 1.0 / scale;
    const double cornerX = tile.x / scale;
    const double cornerY = tile.y / scale;
    double smallest = 2.0;
    smallest = std::min(smallest, distanceToTileWrapX(pointX, pointY, cornerX, cornerY, tileSize));
    smallest = std::min(smallest, distanceToTileWrapX(pointX, pointY, cornerX + 0.5, -cornerY - tileSize, tileSize));
    smallest = std::min(smallest,
                        distanceToTileWrapX(pointX, pointY, cornerX + 0.5, 2.0 - cornerY - tileSize, tileSize));
    return smallest;
}

// The wrap that keeps a tile loaded while the view crosses the antimeridian.
int16_t wrapFor(double centerX, const CanonicalTileID& tile) {
    const double scale = static_cast<double>(1u << tile.z);
    const double tileSize = 1.0 / scale;
    const double tileX = tile.x / scale;
    const double current = distanceToTileSimple(centerX, tileX, tileSize);
    const double left = distanceToTileSimple(centerX, tileX - 1.0, tileSize);
    const double right = distanceToTileSimple(centerX, tileX + 1.0, tileSize);
    const double smallest = std::min({current, left, right});
    if (smallest == right) {
        return 1;
    }
    if (smallest == left) {
        return -1;
    }
    return 0;
}

double integralOfCosXByP(double p, double x1, double x2) {
    constexpr int numPoints = 10;
    double sum = 0;
    const double dx = (x2 - x1) / numPoints;
    for (int i = 0; i < numPoints; i++) {
        const double x = x1 + (i + 0.5) / numPoints * (x2 - x1);
        sum += dx * std::pow(std::cos(x), p);
    }
    return sum;
}

// The parts of GL JS `createCalculateTileZoomFunction(9.314, 3.0)` that depend on the camera alone.
struct TileZoomConstants {
    double pitchTileLoadingBehavior;
    double tileCountPitch0;

    explicit TileZoomConstants(double cameraVerticalFOV)
        : pitchTileLoadingBehavior(2 *
                                   ((maxZoomLevelsOnScreen - 1) /
                                        std::log2(std::cos(util::deg2rad(maxMercatorHorizonAngle - cameraVerticalFOV)) /
                                                  std::cos(util::deg2rad(maxMercatorHorizonAngle))) -
                                    1)),
          tileCountPitch0(2 *
                          integralOfCosXByP(pitchTileLoadingBehavior - 1, 0, util::deg2rad(cameraVerticalFOV / 2))) {}

    static constexpr double maxZoomLevelsOnScreen = 9.314;
    static constexpr double tileCountMaxMinRatio = 3.0;
};

// The zoom a tile is loaded at when the view is pitched.
double calculateTileZoom(const TileZoomConstants& constants,
                         double requestedCenterZoom,
                         double distanceToTile2D,
                         double distanceToTileZ,
                         double distanceToCenter3D,
                         double cameraVerticalFOV) {
    const double pitchTileLoadingBehavior = constants.pitchTileLoadingBehavior;
    const double centerPitch = std::acos(std::min(1.0, distanceToTileZ / distanceToCenter3D));
    const double highestPitch = std::min(util::deg2rad(maxMercatorHorizonAngle),
                                         centerPitch + util::deg2rad(cameraVerticalFOV / 2));
    const double lowestPitch = std::min(highestPitch, centerPitch - util::deg2rad(cameraVerticalFOV / 2));
    const double tileCount = integralOfCosXByP(pitchTileLoadingBehavior - 1, lowestPitch, highestPitch);
    const double thisTilePitch = std::atan(distanceToTile2D / distanceToTileZ);
    const double distanceToTile3D = std::hypot(distanceToTile2D, distanceToTileZ);
    double desired = requestedCenterZoom;
    desired += std::log2(distanceToCenter3D / distanceToTile3D /
                         std::max(0.5, std::cos(util::deg2rad(cameraVerticalFOV / 2))));
    desired += pitchTileLoadingBehavior * std::log2(std::cos(thisTilePitch)) / 2;
    desired -=
        std::log2(std::max(1.0, tileCount / constants.tileCountPitch0 / TileZoomConstants::tileCountMaxMinRatio)) / 2;
    return desired;
}

std::vector<OverscaledTileID> tileCover(const TileCoverParameters& state,
                                        uint8_t z,
                                        const Range<uint8_t> zoomRange,
                                        const std::optional<uint8_t>& overscaledZ) {
    struct Node {
        uint8_t zoom;
        uint32_t x, y;
        int16_t wrap;
        bool fullyVisible;
    };
    struct ResultTile {
        OverscaledTileID id;
        double sqrDist;
    };

    const auto& transform = state.transformState;
    const ProjectionData projection = transform.getProjectionData(UnwrappedTileID(0, 0, 0));
    mat4 inverse;
    // `invert` reports a singular matrix with `true`.
    if (matrix::invert(inverse, projection.mainMatrix)) {
        return {};
    }
    // The globe matrix has no Mercator y flip, which reverses the winding the plane normals come from. The far plane
    // stops at the horizon, so the frustum's corners bound the visible cap and not the space behind the planet.
    const vec4& horizon = projection.clippingPlane;
    const Frustum frustum = Frustum::fromInvProjMatrix(inverse, 1.0, 0.0, /*flippedY=*/true, horizon);

    // Tiles nearer the camera than the center may load finer than the nominal zoom, up to the source's maximum.
    const uint8_t maxZoom = zoomRange.max;
    const uint8_t minZoom = zoomRange.min;
    const uint8_t overscaledZoom = std::max(overscaledZ.value_or(z), maxZoom);
    const double numTiles = std::pow(2.0, z);
    const bool allowVariableZoom = z > 4;

    const Point<double> center = Projection::project(transform.getLatLng(), 1.0 / util::tileSize_D);
    const vec3 centerCoord = {{center.x, center.y, 0.0}};
    const std::optional<vec3> cameraPosition = transform.getFreeCameraOptions().position;
    assert(cameraPosition);
    if (!cameraPosition) {
        return {};
    }
    const vec3& cameraCoord = *cameraPosition;
    const double distanceToCenter2d = std::hypot(centerCoord[0] - cameraCoord[0], centerCoord[1] - cameraCoord[1]);
    const double distanceZ = std::abs(centerCoord[2] - cameraCoord[2]);
    const double distanceToCenter3d = std::hypot(distanceToCenter2d, distanceZ);
    const double requestedCenterZoom = transform.getZoom() + (z - std::floor(transform.getZoom()));
    const double fovDegrees = transform.getFieldOfView() * 180.0 / std::numbers::pi;
    const TileZoomConstants zoomConstants(fovDegrees);
    const double cullingElevation = elevationForTileCulling(transform.getPitch() * 180.0 / std::numbers::pi,
                                                            fovDegrees);

    std::vector<Node> stack;
    std::vector<ResultTile> result;
    stack.reserve(64);
    result.reserve(64);
    stack.push_back({.zoom = 0, .x = 0, .y = 0, .wrap = 0, .fullyVisible = false});

    while (!stack.empty()) {
        Node node = stack.back();
        stack.pop_back();
        const CanonicalTileID tile(node.zoom, node.x, node.y);
        const ConvexVolume volume = tileBoundingVolume(tile, cullingElevation);

        if (!node.fullyVisible) {
            const IntersectionResult intersection = isTileVisible(frustum, volume, horizon);
            if (intersection == IntersectionResult::Separate) continue;
            node.fullyVisible = intersection == IntersectionResult::Contains;
        }

        double desiredZ = z;
        if (allowVariableZoom) {
            const double distToTile2d = distanceToTile2d(cameraCoord[0], cameraCoord[1], tile);
            desiredZ = std::floor(calculateTileZoom(
                zoomConstants, requestedCenterZoom, distToTile2d, distanceZ, distanceToCenter3d, fovDegrees));
        }
        const auto targetZoom = static_cast<uint8_t>(std::clamp(desiredZ, 0.0, static_cast<double>(maxZoom)));

        if (node.zoom >= targetZoom) {
            if (node.zoom < minZoom) {
                continue;
            }
            node.wrap = wrapFor(centerCoord[0], tile);
            // GL JS's sort key: the tile's own x/y against the center at the nominal zoom.
            const double dx = numTiles * centerCoord[0] - 0.5 - node.x;
            const double dy = numTiles * centerCoord[1] - 0.5 - node.y;
            result.push_back(
                {OverscaledTileID(
                     node.zoom == maxZoom ? overscaledZoom : node.zoom, node.wrap, node.zoom, node.x, node.y),
                 dx * dx + dy * dy});
            continue;
        }
        for (int i = 0; i < 4; i++) {
            stack.push_back({.zoom = static_cast<uint8_t>(node.zoom + 1),
                             .x = (node.x << 1) + (i % 2),
                             .y = (node.y << 1) + (i >> 1),
                             .wrap = node.wrap,
                             .fullyVisible = node.fullyVisible});
        }
    }

    std::sort(
        result.begin(), result.end(), [](const ResultTile& a, const ResultTile& b) { return a.sqrDist < b.sqrDist; });
    std::vector<OverscaledTileID> ids;
    ids.reserve(result.size());
    for (const auto& tile : result) {
        ids.push_back(tile.id);
    }
    return ids;
}

} // namespace globe
} // namespace

std::vector<OverscaledTileID> tileCover(const TileCoverParameters& state,
                                        uint8_t z,
                                        const Range<uint8_t> zoomRange,
                                        const std::optional<uint8_t>& overscaledZ) {
    if (state.transformState.isGlobeRendering()) {
        return globe::tileCover(state, z, zoomRange, overscaledZ);
    }

    struct Node {
        AABB aabb;
        uint8_t zoom;
        uint32_t x, y;
        int16_t wrap;
        bool fullyVisible;
    };

    struct ResultTile {
        OverscaledTileID id;
        double sqrDist;
    };

    auto childrenOf = [](const Node& node) -> std::vector<Node> {
        std::vector<Node> children(4);
        for (int i = 0; i < 4; i++) {
            const uint32_t childX = (node.x << 1) + (i % 2);
            const uint32_t childY = (node.y << 1) + (i >> 1);

            children[i] = node;
            children[i].aabb = node.aabb.quadrant(i);
            children[i].zoom = node.zoom + 1;
            children[i].x = childX;
            children[i].y = childY;
        }
        return children;
    };

    const auto& transform = state.transformState;
    const double numTiles = std::pow(2.0, z);
    const double worldSize = Projection::worldSize(transform.getScale());
    const bool allowVariableZoom = transform.getPitch() > state.tileLodPitchThreshold;
    const uint8_t minZoom = allowVariableZoom ? zoomRange.min : z;
    const uint8_t maxZoom = ((state.tileLodMode == TileLodMode::Distance) && allowVariableZoom) ? zoomRange.max : z;
    const uint8_t overscaledZoom = std::max(overscaledZ.value_or(z), maxZoom);
    const bool flippedY = transform.getViewportMode() == ViewportMode::FlippedY;

    const auto centerPoint = TileCoordinate::fromScreenCoordinate(
                                 transform, z, {transform.getSize().width / 2.0, transform.getSize().height / 2.0})
                                 .p;

    const vec3 centerCoord = {{centerPoint.x, centerPoint.y, 0.0}};

    const std::optional<vec3> cameraPosition = transform.getFreeCameraOptions().position;
    assert(cameraPosition);
    if (!cameraPosition) {
        return {};
    }
    const vec3& cameraPositionMercator = *cameraPosition;
    const double nominalScale = std::pow(2.0, z);
    const vec3 cameraCoord = vec3Scale(cameraPositionMercator, nominalScale);
    const double cameraToCenterDistanceMercator = vec3Length(vec3Sub(cameraCoord, centerCoord)) / worldSize;

    const Frustum frustum = Frustum::fromInvProjMatrix(transform.getInvProjectionMatrix(), worldSize, z, flippedY);

    // There should always be a certain number of maximum zoom level tiles
    // surrounding the center location
    assert(state.tileLodMinRadius >= 1);
    const double radiusOfMaxLvlLodInTiles = std::max(1.0, state.tileLodMinRadius);

    const auto newRootTile = [&](int16_t wrap) -> Node {
        return {.aabb = AABB({{wrap * numTiles, 0.0, 0.0}}, {{(wrap + 1) * numTiles, numTiles, 0.0}}),
                .zoom = uint8_t(0),
                .x = uint16_t(0),
                .y = uint16_t(0),
                .wrap = wrap,
                .fullyVisible = false};
    };

    // Perform depth-first traversal on tile tree to find visible tiles
    std::vector<Node> stack;
    std::vector<ResultTile> result;
    stack.reserve(128);

    // World copies shall be rendered three times on both sides from closest to farthest
    for (int i = 1; i <= 3; i++) {
        stack.push_back(newRootTile(-i));
        stack.push_back(newRootTile(i));
    }

    stack.push_back(newRootTile(0));

    while (!stack.empty()) {
        Node node = stack.back();
        stack.pop_back();

        // Use cached visibility information of ancestor nodes
        if (!node.fullyVisible) {
            const IntersectionResult intersection = frustum.intersects(node.aabb);

            if (intersection == IntersectionResult::Separate) continue;

            node.fullyVisible = intersection == IntersectionResult::Contains;
        }

        bool shouldSplitTile;
        if (state.tileLodMode == TileLodMode::Distance) {
            const vec3 camToTileMercator = vec3Scale(node.aabb.distanceXYZ(cameraCoord), 1.0 / worldSize);
            const double distanceToTileMercator = vec3Length(camToTileMercator);
            const double cosPitchToTile = std::max(0.0, camToTileMercator[2] / distanceToTileMercator);
            const double pitchExponent =
                0.5; // 0: constant screen width, 1/2: constant screen area, 1: constant screen height
            double tileScale = std::pow(2.0, node.zoom);
            shouldSplitTile = distanceToTileMercator * tileScale < std::pow(cosPitchToTile, pitchExponent) *
                                                                       cameraToCenterDistanceMercator /
                                                                       state.tileLodScale * nominalScale;
        } else {
            const vec3 distanceXyz = node.aabb.distanceXYZ(centerCoord);
            const double* longestDim = std::max_element(distanceXyz.data(), distanceXyz.data() + distanceXyz.size());
            assert(longestDim);

            // We're using distance based heuristics to determine if a tile should
            // be split into quadrants or not. radiusOfMaxLvlLodInTiles defines that
            // there's always a certain number of maxLevel tiles next to the map
            // center. Using the fact that a parent node in quadtree is twice the
            // size of its children (per dimension) we can define distance
            // thresholds for each relative level:
            // f(k) = offset + 2 + 4 + 8 + 16 + ... + 2^k
            // This is the same as:
            // f(k) = offset + 2^(k+1)-2
            const double distToSplit = radiusOfMaxLvlLodInTiles + (1 << (maxZoom - node.zoom)) - 2;
            shouldSplitTile = *longestDim * state.tileLodScale < distToSplit;
        }

        // Have we reached the target depth or is the tile too far away to be any split further?
        if (node.zoom == maxZoom || (!shouldSplitTile && node.zoom >= minZoom)) {
            // Perform precise intersection test between the frustum and aabb.
            // This will cull < 1% false positives missed by the original test
            if (node.fullyVisible || frustum.intersectsPrecise(node.aabb, true) != IntersectionResult::Separate) {
                const OverscaledTileID id = {
                    node.zoom == maxZoom ? overscaledZoom : node.zoom, node.wrap, node.zoom, node.x, node.y};
                vec3 coordToLoadFirst = (state.tileLodMode == TileLodMode::Distance) ? cameraCoord : centerCoord;
                const double dx = node.wrap * numTiles + node.x + 0.5 - coordToLoadFirst[0];
                const double dy = node.y + 0.5 - coordToLoadFirst[1];

                result.push_back({id, dx * dx + dy * dy});
            }
        } else {
            std::vector<Node> children = childrenOf(node);
            stack.insert(stack.end(), children.begin(), children.end());
        }
    }

    // Sort results by distance
    std::sort(
        result.begin(), result.end(), [](const ResultTile& a, const ResultTile& b) { return a.sqrDist < b.sqrDist; });

    std::vector<OverscaledTileID> ids;
    ids.reserve(result.size());

    for (const auto& tile : result) {
        ids.push_back(tile.id);
    }

    return ids;
}

std::vector<UnwrappedTileID> tileCover(const LatLngBounds& bounds_, uint8_t z) {
    if (bounds_.isEmpty() || bounds_.south() > util::LATITUDE_MAX || bounds_.north() < -util::LATITUDE_MAX) {
        return {};
    }

    const LatLngBounds bounds = LatLngBounds::hull({std::max(bounds_.south(), -util::LATITUDE_MAX), bounds_.west()},
                                                   {std::min(bounds_.north(), util::LATITUDE_MAX), bounds_.east()});

    return tileCover(Projection::project(bounds.northwest(), z),
                     Projection::project(bounds.northeast(), z),
                     Projection::project(bounds.southeast(), z),
                     Projection::project(bounds.southwest(), z),
                     Projection::project(bounds.center(), z),
                     z);
}

std::vector<UnwrappedTileID> tileCover(const Geometry<double>& geometry, uint8_t z) {
    std::vector<UnwrappedTileID> result;
    TileCover tc(geometry, z, true);
    while (tc.hasNext()) {
        result.push_back(*tc.next());
    };

    return result;
}

// Taken from https://github.com/mapbox/sphericalmercator#xyzbbox-zoom-tms_style-srs
// Computes the projected tiles for the lower left and upper right points of the bounds
// and uses that to compute the tile cover count
uint64_t tileCount(const LatLngBounds& bounds, uint8_t zoom) noexcept {
    if (zoom == 0) {
        return 1;
    }
    const auto sw = Projection::project(bounds.southwest(), zoom);
    const auto ne = Projection::project(bounds.northeast(), zoom);
    const auto maxTile = std::pow(2.0, zoom);
    const auto x1 = floor(sw.x);
    const auto x2 = ceil(ne.x) - 1;
    const auto y1 = util::clamp(floor(sw.y), 0.0, maxTile - 1);
    const auto y2 = util::clamp(floor(ne.y), 0.0, maxTile - 1);

    const auto dx = x1 > x2 ? (maxTile - x1) + x2 : x2 - x1;
    const auto dy = y1 - y2;
    return static_cast<uint64_t>((dx + 1) * (dy + 1));
}

uint64_t tileCount(const Geometry<double>& geometry, uint8_t z) {
    uint64_t tileCount = 0;

    TileCover tc(geometry, z, true);
    while (tc.next()) {
        tileCount++;
    };
    return tileCount;
}

TileCover::TileCover(const LatLngBounds& bounds_, uint8_t z) {
    LatLngBounds bounds = LatLngBounds::hull({std::max(bounds_.south(), -util::LATITUDE_MAX), bounds_.west()},
                                             {std::min(bounds_.north(), util::LATITUDE_MAX), bounds_.east()});

    if (bounds.isEmpty() || bounds.south() > util::LATITUDE_MAX || bounds.north() < -util::LATITUDE_MAX) {
        bounds = LatLngBounds::world();
    }

    const auto sw = Projection::project(bounds.southwest(), z);
    const auto ne = Projection::project(bounds.northeast(), z);
    const auto se = Projection::project(bounds.southeast(), z);
    const auto nw = Projection::project(bounds.northwest(), z);

    const Polygon<double> p({{sw, nw, ne, se, sw}});
    impl = std::make_unique<TileCover::Impl>(z, p, false);
}

TileCover::TileCover(const Geometry<double>& geom, uint8_t z, bool project /* = true*/)
    : impl(std::make_unique<TileCover::Impl>(z, geom, project)) {}

TileCover::~TileCover() = default;

std::optional<UnwrappedTileID> TileCover::next() {
    return impl->next();
}

bool TileCover::hasNext() {
    return impl->hasNext();
}

} // namespace util
} // namespace mln
