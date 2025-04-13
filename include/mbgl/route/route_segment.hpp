
#include <mbgl/util/geometry.hpp>
#include <mbgl/util/color.hpp>
#include <memory>

#pragma once

namespace mbgl {
namespace route {

struct RouteSegmentOptions {
    Color color = Color(1.0f, 1.f, 1.0f, 1.0f);
    LineString<double> geometry;
    uint32_t priority = 0;
};

class RouteSegment {
public:
    RouteSegment() = default;
    RouteSegment(const RouteSegmentOptions& routeSegOptions,
                 const LineString<double>& routeGeometry,
                 const std::vector<double>& routeGeomDistances,
                 double routeTotalDistance);
    std::vector<double> getNormalizedPositions() const;
    RouteSegmentOptions getRouteSegmentOptions() const;

    ~RouteSegment();

private:
    RouteSegmentOptions options_;
    std::vector<double> normalizedPositions_;
};
} // namespace route
} // namespace mbgl
