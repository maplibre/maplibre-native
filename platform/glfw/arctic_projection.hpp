#pragma once

#include <mln/util/crs.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <span>

namespace mln {
namespace epsg3413 {

inline constexpr double semiMajorAxis = 6378137;
inline constexpr double flattening = 1 / 298.257223563;
inline constexpr double e2 = flattening * (2 - flattening);
inline constexpr double centralLongitude = -45;
inline constexpr double trueScaleLatitude = 70;
inline constexpr double radians = std::numbers::pi / 180;

/// Snyder's t, the conformal latitude term, for a latitude in radians.
inline double t(double latitude) {
    const double e = std::sqrt(e2);
    const double sinLatitude = std::sin(latitude);
    return std::tan((std::numbers::pi / 4) - (latitude / 2)) /
           std::pow((1 - (e * sinLatitude)) / (1 + (e * sinLatitude)), e / 2);
}

/// The distance from the pole in meters per unit of t.
inline double scale() {
    const double latitude = trueScaleLatitude * radians;
    const double sinLatitude = std::sin(latitude);
    const double m = std::cos(latitude) / std::sqrt(1 - (e2 * sinLatitude * sinLatitude));
    return semiMajorAxis * m / t(latitude);
}

} // namespace epsg3413

/// EPSG:3413, NSIDC's north polar stereographic projection of the WGS 84 ellipsoid, true to scale at 70 N with 45 W
/// pointing down, on the grid NASA GIBS serves it in (tile 0/0/0 spans 8388608 m from x, y = -4194304, 4194304).
/// Formulas from Snyder, Map Projections: A Working Manual (1987), 21-34 and 21-38 to 21-41.
inline CRS epsg3413Projection() {
    using namespace epsg3413;
    return {.name = "EPSG:3413",
            .forward =
                [rhoPerT = scale()](std::span<const std::array<double, 2>> lngLats,
                                    std::span<std::array<double, 2>> positions) {
                    for (std::size_t i = 0; i < lngLats.size(); ++i) {
                        const double rho = rhoPerT * t(lngLats[i][1] * radians);
                        const double angle = (lngLats[i][0] - centralLongitude) * radians;
                        positions[i] = {rho * std::sin(angle), -rho * std::cos(angle)};
                    }
                },
            .inverse =
                [rhoPerT = scale()](std::span<const std::array<double, 2>> positions,
                                    std::span<std::array<double, 2>> lngLats) {
                    constexpr double e4 = e2 * e2;
                    constexpr double e6 = e4 * e2;
                    constexpr double e8 = e6 * e2;
                    for (std::size_t i = 0; i < positions.size(); ++i) {
                        const auto& [x, y] = positions[i];
                        const double chi = (std::numbers::pi / 2) - (2 * std::atan(std::hypot(x, y) / rhoPerT));
                        const double latitude =
                            chi + (((e2 / 2) + (5 * e4 / 24) + (e6 / 12) + (13 * e8 / 360)) * std::sin(2 * chi)) +
                            (((7 * e4 / 48) + (29 * e6 / 240) + (811 * e8 / 11520)) * std::sin(4 * chi)) +
                            (((7 * e6 / 120) + (81 * e8 / 1120)) * std::sin(6 * chi)) +
                            ((4279 * e8 / 161280) * std::sin(8 * chi));
                        lngLats[i] = {centralLongitude + (std::atan2(x, -y) / radians), latitude / radians};
                    }
                },
            .tileMatrixSet = {.origin = {-4194304, 4194304}, .extentAtZoom0 = 8388608}};
}

} // namespace mln
