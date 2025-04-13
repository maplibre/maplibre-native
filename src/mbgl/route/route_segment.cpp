#include <boost/qvm/mat_operations.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/route/route_segment.hpp>
#include <mbgl/style/expression/dsl.hpp>

namespace mbgl {
namespace route {

namespace {
// Function to check if point A is between points B and C
bool isPointBetween(const mbgl::Point<double>& A,
                    const mbgl::Point<double>& B,
                    const mbgl::Point<double>& C,
                    bool quickDirty = false) {
    // Check if the points are collinear (on the same line)
    double crossProduct = (C.y - B.y) * (A.x - B.x) - (C.x - B.x) * (A.y - B.y);

    if (std::abs(crossProduct) > 1e-9) { // Using a small epsilon for floating-point comparison
        return false;                    // Not collinear, so A cannot be between B and C
    }

    // Check if A is within the bounding box of B and C.  This is a simplified version.
    // A more robust check would involve projecting A onto the line BC and checking
    // if the projection lies within the segment BC.  The bounding box check is sufficient
    // for many common cases and is less computationally expensive.
    if (quickDirty) {
        double minX = std::min(B.x, C.x);
        double maxX = std::max(B.x, C.x);
        double minY = std::min(B.y, C.y);
        double maxY = std::max(B.y, C.y);

        return (A.x >= minX && A.x <= maxX && A.y >= minY && A.y <= maxY);
    }

    // More robust check (projection method - commented out for brevity, but recommended):
    double dotProduct = (A.x - B.x) * (C.x - B.x) + (A.y - B.y) * (C.y - B.y);
    double squaredLengthBC = (C.x - B.x) * (C.x - B.x) + (C.y - B.y) * (C.y - B.y);

    if (squaredLengthBC == 0) {            // B and C are the same point
        return (A.x == B.x && A.y == B.y); // A must be equal to B (and C)
    }

    double t = dotProduct / squaredLengthBC;

    return (t >= 0 && t <= 1);
}
} // namespace

RouteSegment::RouteSegment(const RouteSegmentOptions& routeSegOptions,
                           const LineString<double>& routeGeometry,
                           const std::vector<double>& routeGeomDistances,
                           double routeTotalDistance)
    : options_(routeSegOptions) {
    // calculate the normalized points and the expressions
    double currDist = 0.0;
    for (const auto& pt : routeSegOptions.geometry) {
        bool ptIntersectionFound = false;
        for (size_t i = 1; i < routeGeometry.size(); ++i) {
            const mbgl::Point<double>& pt1 = routeGeometry[i - 1];
            const mbgl::Point<double>& pt2 = pt;
            const mbgl::Point<double>& pt3 = routeGeometry[i];

            if (isPointBetween(pt2, pt1, pt3)) {
                double partialDist = mbgl::util::dist<double>(pt1, pt2);
                currDist += partialDist;
                ptIntersectionFound = true;
                break;
            } else {
                currDist += routeGeomDistances[i - 1];
            }
        }

        if (ptIntersectionFound) {
            double normalizedDist = currDist / routeTotalDistance;
            normalizedPositions_.push_back(normalizedDist);
        }

        currDist = 0.0;
    }
}

std::vector<double> RouteSegment::getNormalizedPositions() const {
    return normalizedPositions_;
}

RouteSegmentOptions RouteSegment::getRouteSegmentOptions() const {
    return options_;
}

RouteSegment::~RouteSegment() {}
} // namespace route
} // namespace mbgl
