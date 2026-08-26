#include <mln/util/bounding_volumes.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>

namespace mln {
namespace {

vec3 toVec3(const vec4& v) noexcept {
    return vec3{{v[0], v[1], v[2]}};
}

double vec4Dot(const vec4& a, const vec4& b) noexcept {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

std::optional<double> rayPlaneIntersection(const vec3& origin, const vec3& direction, const vec4& plane) noexcept {
    const vec3 normal = {{plane[0], plane[1], plane[2]}};
    const double along = vec3Dot(direction, normal);
    if (along == 0) {
        return std::nullopt;
    }
    return (-vec3Dot(origin, normal) - plane[3]) / along;
}

vec4 planeThrough(const vec3& p0, const vec3& p1, const vec3& p2) noexcept {
    const vec3 n = vec3Normalize(vec3Cross(vec3Sub(p0, p1), vec3Sub(p2, p1)));
    return {{n[0], n[1], n[2], -vec3Dot(n, p1)}};
}

// The distance from the near plane to the point of the horizon circle farthest along the view direction, if the
// camera is not looking straight down.
std::optional<double> idealNearFarPlaneDistance(const vec4& horizonPlane, const vec4& nearPlane) noexcept {
    const vec3 view = {{nearPlane[0], nearPlane[1], nearPlane[2]}};
    const double horizonLength = vec3Length({{horizonPlane[0], horizonPlane[1], horizonPlane[2]}});
    const vec3 horizonNormal = vec3Scale({{horizonPlane[0], horizonPlane[1], horizonPlane[2]}}, 1.0 / horizonLength);
    const double horizonDistance = horizonPlane[3] / horizonLength;
    const vec3 projectedView = vec3Sub(view, vec3Scale(horizonNormal, vec3Dot(view, horizonNormal)));
    const double projectedLength = vec3Length(projectedView);
    if (projectedLength <= 0) {
        return std::nullopt;
    }
    const double circleRadius = std::sqrt(1.0 - horizonDistance * horizonDistance);
    const vec3 circleCenter = vec3Scale(horizonNormal, -horizonDistance);
    const vec3 farthest = vec3Add(circleCenter, vec3Scale(projectedView, circleRadius / projectedLength));
    return vec3Dot(view, farthest) + nearPlane[3];
}

// Moves the far corners in along the frustum's edges until the far plane reaches no further than the horizon;
// port of GL JS `adjustFarPlaneByHorizonPlane`. The near corners are the first four.
void adjustFarPlaneByHorizonPlane(std::array<vec4, 8>& corners,
                                  const std::array<int, 3>& nearPlaneIndices,
                                  const vec4& horizonPlane) noexcept {
    constexpr std::size_t nearOffset = 0;
    constexpr std::size_t farOffset = 4;
    std::array<double, 4> rayLengths{};
    std::array<vec3, 4> rayDirections{};
    double maxDistance = 0;
    for (std::size_t i = 0; i < 4; i++) {
        const vec3 ray = vec3Sub(toVec3(corners[i + farOffset]), toVec3(corners[i + nearOffset]));
        rayLengths[i] = vec3Length(ray);
        rayDirections[i] = vec3Scale(ray, 1.0 / rayLengths[i]);
    }
    for (std::size_t i = 0; i < 4; i++) {
        const auto distance = rayPlaneIntersection(toVec3(corners[i + nearOffset]), rayDirections[i], horizonPlane);
        // Rays parallel to the horizon or pointing away from it keep their length.
        maxDistance = std::max(maxDistance, distance && *distance >= 0 ? *distance : rayLengths[i]);
    }
    const vec4 nearPlane = planeThrough(toVec3(corners[nearPlaneIndices[0]]),
                                        toVec3(corners[nearPlaneIndices[1]]),
                                        toVec3(corners[nearPlaneIndices[2]]));
    if (const auto ideal = idealNearFarPlaneDistance(horizonPlane, nearPlane)) {
        // The rays make the same angle with the near plane at all four corners.
        maxDistance = std::min(maxDistance,
                               *ideal / vec3Dot(rayDirections[0], {{nearPlane[0], nearPlane[1], nearPlane[2]}}));
    }
    for (std::size_t i = 0; i < 4; i++) {
        const vec3 far = vec3Add(toVec3(corners[i + nearOffset]),
                                 vec3Scale(rayDirections[i], std::min(maxDistance, rayLengths[i])));
        corners[i + farOffset] = {{far[0], far[1], far[2], 1.0}};
    }
}

template <size_t N>
Point<double> ProjectPointsToAxis(const std::array<vec3, N>& points, const vec3& origin, const vec3& axis) noexcept {
    double min = std::numeric_limits<double>::max();
    double max = -std::numeric_limits<double>::max();

    for (const vec3& point : points) {
        const double projectedPoint = vec3Dot(vec3Sub(point, origin), axis);
        min = std::min(projectedPoint, min);
        max = std::max(projectedPoint, max);
    }

    return {min, max};
}

} // namespace

namespace util {

AABB::AABB() noexcept
    : min({{0, 0, 0}}),
      max({{0, 0, 0}}) {}

AABB::AABB(const vec3& min_, const vec3& max_) noexcept
    : min(min_),
      max(max_) {}

constexpr vec3 AABB::closestPoint(const vec3& point) const noexcept {
    return {{std::max(std::min(max[0], point[0]), min[0]),
             std::max(std::min(max[1], point[1]), min[1]),
             std::max(std::min(max[2], point[2]), min[2])}};
}

vec3 AABB::distanceXYZ(const vec3& point) const noexcept {
    vec3 vec = vec3Sub(closestPoint(point), point);

    vec[0] = std::abs(vec[0]);
    vec[1] = std::abs(vec[1]);
    vec[2] = std::abs(vec[2]);

    return vec;
}

AABB AABB::quadrant(int idx) const noexcept {
    assert(idx >= 0 && idx < 4);
    vec3 quadrantMin = min;
    vec3 quadrantMax = max;
    const double xCenter = 0.5 * (max[0] + min[0]);
    const double yCenter = 0.5 * (max[1] + min[1]);

    // This aabb is split into 4 quadrants. For each axis define in which side
    // of the split "idx" is The result for indices 0, 1, 2, 3 is: { 0, 0 }, {
    // 1, 0 }, { 0, 1 }, { 1, 1 }
    const std::array<int, 4> xSplit = {{0, 1, 0, 1}};
    const std::array<int, 4> ySplit = {{0, 0, 1, 1}};

    quadrantMin[0] = xSplit[idx] ? xCenter : quadrantMin[0];
    quadrantMax[0] = xSplit[idx] ? quadrantMax[0] : xCenter;

    quadrantMin[1] = ySplit[idx] ? yCenter : quadrantMin[1];
    quadrantMax[1] = ySplit[idx] ? quadrantMax[1] : yCenter;

    return {quadrantMin, quadrantMax};
}

bool AABB::intersects(const AABB& aabb) const noexcept {
    if (min[0] > aabb.max[0] || aabb.min[0] > max[0]) return false;
    if (min[1] > aabb.max[1] || aabb.min[1] > max[1]) return false;
    if (min[2] > aabb.max[2] || aabb.min[2] > max[2]) return false;
    return true;
}

bool AABB::operator==(const AABB& aabb) const noexcept {
    return min == aabb.min && max == aabb.max;
}

bool AABB::operator!=(const AABB& aabb) const noexcept {
    return !(*this == aabb);
}

// Named index values for frustum::points array
enum {
    near_tl = 0,
    near_tr = 1,
    near_br = 2,
    near_bl = 3,
    far_tl = 4,
    far_tr = 5,
    far_br = 6,
    far_bl = 7,
};

Frustum::Frustum(const std::array<vec3, 8>& points_, const std::array<vec4, 6>& planes_)
    : points(points_),
      planes(planes_) {
    const Point<double> xBounds = ProjectPointsToAxis(points, {{0, 0, 0}}, {{1, 0, 0}});
    const Point<double> yBounds = ProjectPointsToAxis(points, {{0, 0, 0}}, {{0, 1, 0}});
    const Point<double> zBounds = ProjectPointsToAxis(points, {{0, 0, 0}}, {{0, 0, 1}});

    bounds = AABB({{xBounds.x, yBounds.x, zBounds.x}}, {{xBounds.y, yBounds.y, zBounds.y}});

    // Precompute a set of separating axis candidates for precise intersection
    // tests. Remaining axes not covered in basic intersection tests are: axis[]
    // = (edges of aabb) x (edges of frustum)
    std::array<vec3, 6> frustumEdges = {{vec3Sub(points[near_br], points[near_bl]),
                                         vec3Sub(points[near_tl], points[near_bl]),
                                         vec3Sub(points[far_tl], points[near_tl]),
                                         vec3Sub(points[far_tr], points[near_tr]),
                                         vec3Sub(points[far_br], points[near_br]),
                                         vec3Sub(points[far_bl], points[near_bl])}};

    for (size_t i = 0; i < frustumEdges.size(); i++) {
        // Cross product [1, 0, 0] x [a, b, c] == [0, -c, b]
        // Cross product [0, 1, 0] x [a, b, c] == [c, 0, -a]
        const vec3 axis0 = {{0.0, -frustumEdges[i][2], frustumEdges[i][1]}};
        const vec3 axis1 = {{frustumEdges[i][2], 0.0, -frustumEdges[i][0]}};

        projections[i * 2] = {.axis = axis0, .projection = ProjectPointsToAxis(points, points[0], axis0)};
        projections[i * 2 + 1] = {.axis = axis1, .projection = ProjectPointsToAxis(points, points[0], axis1)};
    }
}

Frustum Frustum::fromInvProjMatrix(
    const mat4& invProj, double worldSize, double zoom, bool flippedY, const std::optional<vec4>& horizonPlane) {
    // Define frustum corner points in normalized clip space
    std::array<vec4, 8> cornerCoords = {{vec4{{-1.0, 1.0, -1.0, 1.0}},
                                         vec4{{1.0, 1.0, -1.0, 1.0}},
                                         vec4{{1.0, -1.0, -1.0, 1.0}},
                                         vec4{{-1.0, -1.0, -1.0, 1.0}},
                                         vec4{{-1.0, 1.0, 1.0, 1.0}},
                                         vec4{{1.0, 1.0, 1.0, 1.0}},
                                         vec4{{1.0, -1.0, 1.0, 1.0}},
                                         vec4{{-1.0, -1.0, 1.0, 1.0}}}};

    const double scale = std::pow(2.0, zoom);

    // Transform points to tile space
    for (auto& coord : cornerCoords) {
        matrix::transformMat4(coord, coord, invProj);
        for (auto& component : coord) component *= 1.0 / coord[3] / worldSize * scale;
    }

    std::array<vec3i, 6> frustumPlanePointIndices = {{
        vec3i{{near_bl, near_br, far_br}},  // bottom
        vec3i{{near_tl, near_bl, far_bl}},  // left
        vec3i{{near_br, near_tr, far_tr}},  // right
        vec3i{{near_tl, far_tl, far_tr}},   // top
        vec3i{{near_tl, near_tr, near_br}}, // near
        vec3i{{far_br, far_tr, far_tl}}     // far
    }};

    if (flippedY) {
        std::ranges::for_each(frustumPlanePointIndices, [](vec3i& tri) { std::swap(tri[1], tri[2]); });
    }

    if (horizonPlane) {
        adjustFarPlaneByHorizonPlane(cornerCoords, frustumPlanePointIndices[4], *horizonPlane);
    }

    std::array<vec4, 6> frustumPlanes;

    for (std::size_t i = 0; i < frustumPlanePointIndices.size(); i++) {
        const vec3i indices = frustumPlanePointIndices[i];
        frustumPlanes[i] = planeThrough(
            toVec3(cornerCoords[indices[0]]), toVec3(cornerCoords[indices[1]]), toVec3(cornerCoords[indices[2]]));
    }

    std::array<vec3, 8> frustumPoints;

    for (size_t i = 0; i < cornerCoords.size(); i++) frustumPoints[i] = toVec3(cornerCoords[i]);

    return {frustumPoints, frustumPlanes};
}

IntersectionResult Frustum::intersects(const AABB& aabb) const {
    // Execute separating axis test between two convex objects to find intersections
    // Each frustum plane together with 3 major axes define the separating axes
    // This implementation is conservative as it's not checking all possible axes.
    // False positive rate is ~0.5% of all cases (see intersectsPrecise).
    // Note: test only 4 points as both min and max points have zero elevation
    assert(aabb.min[2] == 0.0 && aabb.max[2] == 0.0);

    if (!bounds.intersects(aabb)) return IntersectionResult::Separate;

    const std::array<vec4, 4> aabbPoints = {{
        vec4{{aabb.min[0], aabb.min[1], 0.0, 1.0}},
        vec4{{aabb.max[0], aabb.min[1], 0.0, 1.0}},
        vec4{{aabb.max[0], aabb.max[1], 0.0, 1.0}},
        vec4{{aabb.min[0], aabb.max[1], 0.0, 1.0}},
    }};

    bool fullyInside = true;

    const auto epsilon = 1e-10;

    for (const vec4& plane : planes) {
        size_t pointsInside = 0;

        pointsInside += vec4Dot(plane, aabbPoints[0]) >= -epsilon;
        pointsInside += vec4Dot(plane, aabbPoints[1]) >= -epsilon;
        pointsInside += vec4Dot(plane, aabbPoints[2]) >= -epsilon;
        pointsInside += vec4Dot(plane, aabbPoints[3]) >= -epsilon;

        if (!pointsInside) {
            // Separating axis found, no intersection
            return IntersectionResult::Separate;
        }

        if (pointsInside != aabbPoints.size()) fullyInside = false;
    }

    return fullyInside ? IntersectionResult::Contains : IntersectionResult::Intersects;
}

IntersectionResult Frustum::intersectsPrecise(const AABB& aabb, bool edgeCasesOnly) const {
    if (!edgeCasesOnly) {
        IntersectionResult result = intersects(aabb);

        if (result == IntersectionResult::Separate) return result;
    }

    const std::array<vec3, 4> aabbPoints = {{vec3{{aabb.min[0], aabb.min[1], 0.0}},
                                             vec3{{aabb.max[0], aabb.min[1], 0.0}},
                                             vec3{{aabb.max[0], aabb.max[1], 0.0}},
                                             vec3{{aabb.min[0], aabb.max[1], 0.0}}}};

    // For a precise SAT-test all edge cases needs to be covered
    // Projections of the frustum on separating axis candidates have been precomputed already
    for (const Projection& proj : projections) {
        Point<double> projectedAabb = ProjectPointsToAxis(aabbPoints, points[0], proj.axis);
        const Point<double>& projectedFrustum = proj.projection;

        if (projectedFrustum.y < projectedAabb.x || projectedFrustum.x > projectedAabb.y) {
            return IntersectionResult::Separate;
        }
    }

    return IntersectionResult::Intersects;
}

} // namespace util
} // namespace mln
