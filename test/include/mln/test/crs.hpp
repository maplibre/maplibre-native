#pragma once

#include <mln/util/crs.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <span>
#include <string>

namespace mln {
namespace test {

/// A converter that takes each position on its own through `convert`.
template <class Convert>
auto eachPosition(Convert convert) {
    return [convert](std::span<const std::array<double, 2>> in, std::span<std::array<double, 2>> out) {
        std::ranges::transform(in, out.begin(), convert);
    };
}

inline std::array<double, 2> unchanged(const std::array<double, 2>& position) {
    return position;
}

/// A north polar stereographic projection of a sphere, with a tile matrix set the size of EPSG:3413's.
inline CRS northPolarStereographic(std::string name) {
    constexpr double radius = 6378137;
    return {.name = std::move(name),
            .forward = eachPosition([](const std::array<double, 2>& lngLat) {
                const double lambda = lngLat[0] * std::numbers::pi / 180;
                const double rho = 2 * radius * std::tan(std::numbers::pi / 4 - lngLat[1] * std::numbers::pi / 360);
                return std::array<double, 2>{rho * std::sin(lambda), -rho * std::cos(lambda)};
            }),
            .inverse = eachPosition([](const std::array<double, 2>& position) {
                const double rho = std::hypot(position[0], position[1]);
                const double latitude = 90 - std::atan(rho / (2 * radius)) * 360 / std::numbers::pi;
                return std::array<double, 2>{std::atan2(position[0], -position[1]) * 180 / std::numbers::pi, latitude};
            }),
            .tileMatrixSet = {.origin = {-4194304, 4194304}, .extentAtZoom0 = 8388608}};
}

/// GL JS's test polar stereographic projection: EPSG:3413's true scale at 70 N and central meridian at 45 W, on a
/// sphere, with EPSG:3413's tile matrix set.
inline CRS polarStereographic(std::string name) {
    constexpr double degrees = 180 / std::numbers::pi;
    static const double scale = 6378137 * (1 + std::sin(70 / degrees));
    constexpr double centralLongitude = -45;
    return {.name = std::move(name),
            .forward = eachPosition([](const std::array<double, 2>& lngLat) {
                const double rho = scale * std::tan(std::numbers::pi / 4 - lngLat[1] / degrees / 2);
                const double angle = (lngLat[0] - centralLongitude) / degrees;
                return std::array<double, 2>{rho * std::sin(angle), -rho * std::cos(angle)};
            }),
            .inverse = eachPosition([](const std::array<double, 2>& position) {
                return std::array<double, 2>{
                    centralLongitude + std::atan2(position[0], -position[1]) * degrees,
                    90 - 2 * std::atan(std::hypot(position[0], position[1]) / scale) * degrees};
            }),
            .tileMatrixSet = {.origin = {-4194304, 4194304}, .extentAtZoom0 = 8388608}};
}

/// Identity coordinates over a square twice as wide as the world, so a center can go past 180 degrees.
inline CRS wideIdentity(std::string name) {
    return {.name = std::move(name),
            .forward = eachPosition(unchanged),
            .inverse = eachPosition(unchanged),
            .tileMatrixSet = {.origin = {-360, 360}, .extentAtZoom0 = 720}};
}

/// The identity, with an inverse that has no answer outside the disc inscribed in its tile matrix set.
inline CRS disc(std::string name) {
    return {.name = std::move(name),
            .forward = eachPosition(unchanged),
            .inverse = eachPosition([](const std::array<double, 2>& position) {
                if (std::hypot(position[0], position[1]) > 90) {
                    return std::array<double, 2>{std::numeric_limits<double>::quiet_NaN(),
                                                 std::numeric_limits<double>::quiet_NaN()};
                }
                return position;
            }),
            .tileMatrixSet = {.origin = {-90, 90}, .extentAtZoom0 = 180}};
}

} // namespace test
} // namespace mln
