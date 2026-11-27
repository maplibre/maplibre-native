#pragma once

#include <mln/util/crs.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <span>
#include <string>
#include <utility>

namespace mln {
namespace test {

/// The formulas and constants of Ordnance Survey's "A Guide to Coordinate Systems in Great Britain", V3.6 (2020):
/// annex A for the constants, B for Cartesian coordinates, C for the transverse Mercator projection, section 6.6
/// for the Helmert transformation.
namespace national_grid {

inline constexpr double radiansPerDegree = std::numbers::pi / 180;
inline constexpr double radiansPerArcSecond = radiansPerDegree / 3600;

struct Ellipsoid {
    double a;
    double b;

    constexpr double e2() const { return ((a * a) - (b * b)) / (a * a); }
};

inline constexpr Ellipsoid airy1830{.a = 6377563.396, .b = 6356256.909};
inline constexpr Ellipsoid grs80{.a = 6378137, .b = 6356752.3141};

struct LatLngHeight {
    double latitude;
    double longitude;
    double height;
};

/// Latitude and longitude in degrees and height above the ellipsoid in meters to Cartesian coordinates (B2 to B5).
inline std::array<double, 3> cartesianFromLatLngHeight(const Ellipsoid& ellipsoid, const LatLngHeight& position) {
    const double phi = position.latitude * radiansPerDegree;
    const double lambda = position.longitude * radiansPerDegree;
    const double e2 = ellipsoid.e2();
    const double nu = ellipsoid.a / std::sqrt(1 - (e2 * std::sin(phi) * std::sin(phi)));
    return {(nu + position.height) * std::cos(phi) * std::cos(lambda),
            (nu + position.height) * std::cos(phi) * std::sin(lambda),
            (((1 - e2) * nu) + position.height) * std::sin(phi)};
}

/// Cartesian coordinates to latitude, longitude and height (B6 to B9), iterating until the latitude moves by less
/// than a micrometer.
inline LatLngHeight latLngHeightFromCartesian(const Ellipsoid& ellipsoid, const std::array<double, 3>& xyz) {
    constexpr double latitudeTolerance = 1e-6 / 6378137;
    const auto& [x, y, z] = xyz;
    const double e2 = ellipsoid.e2();
    const double p = std::hypot(x, y);
    const auto nuAt = [&](double phi) {
        return ellipsoid.a / std::sqrt(1 - (e2 * std::sin(phi) * std::sin(phi)));
    };
    double phi = std::atan(z / (p * (1 - e2)));
    double change = 0;
    do {
        const double next = std::atan((z + (e2 * nuAt(phi) * std::sin(phi))) / p);
        change = next - phi;
        phi = next;
    } while (std::abs(change) >= latitudeTolerance);
    return {.latitude = phi / radiansPerDegree,
            .longitude = std::atan2(y, x) / radiansPerDegree,
            .height = (p / std::cos(phi)) - nuAt(phi)};
}

/// A seven-parameter Helmert transformation of Cartesian coordinates (section 6.2, equation 3).
struct Helmert {
    std::array<double, 3> translation;
    double scalePpm;
    std::array<double, 3> rotationArcSeconds;

    std::array<double, 3> operator()(const std::array<double, 3>& xyz) const {
        const double s = 1 + (scalePpm * 1e-6);
        const double rx = rotationArcSeconds[0] * radiansPerArcSecond;
        const double ry = rotationArcSeconds[1] * radiansPerArcSecond;
        const double rz = rotationArcSeconds[2] * radiansPerArcSecond;
        const auto& [x, y, z] = xyz;
        return {translation[0] + (s * x) - (rz * y) + (ry * z),
                translation[1] + (rz * x) + (s * y) - (rx * z),
                translation[2] - (ry * x) + (rx * y) + (s * z)};
    }

    /// The reverse transformation, as section 6.2 gives it: every parameter negated.
    Helmert reversed() const {
        return {.translation = {-translation[0], -translation[1], -translation[2]},
                .scalePpm = -scalePpm,
                .rotationArcSeconds = {-rotationArcSeconds[0], -rotationArcSeconds[1], -rotationArcSeconds[2]}};
    }
};

/// WGS84 or ETRS89, which differ negligibly here, to OSGB36 (section 6.6, table 4). It is up to 3.5 m (95%) from the
/// National Grid's definitive transformation, OSTN15.
inline constexpr Helmert etrs89ToOsgb36{.translation = {-446.448, 125.157, -542.060},
                                        .scalePpm = 20.4894,
                                        .rotationArcSeconds = {-0.1502, -0.2470, -0.8421}};

/// A transverse Mercator projection of an ellipsoid (annex C).
struct TransverseMercator {
    Ellipsoid ellipsoid;
    double scale;
    double originLatitude;
    double centralLongitude;
    double falseEasting;
    double falseNorthing;

    /// Latitude and longitude in degrees to easting and northing in meters (C1 to C5).
    std::array<double, 2> forward(double latitude, double longitude) const {
        const double phi = latitude * radiansPerDegree;
        const double sinPhi = std::sin(phi);
        const double cosPhi = std::cos(phi);
        const double tan2 = std::tan(phi) * std::tan(phi);
        const auto [nu, rho, eta2] = radii(phi);
        const double i = meridianArc(phi) + falseNorthing;
        const double ii = nu / 2 * sinPhi * cosPhi;
        const double iii = nu / 24 * sinPhi * std::pow(cosPhi, 3) * (5 - tan2 + (9 * eta2));
        const double iiia = nu / 720 * sinPhi * std::pow(cosPhi, 5) * (61 - (58 * tan2) + (tan2 * tan2));
        const double iv = nu * cosPhi;
        const double v = nu / 6 * std::pow(cosPhi, 3) * ((nu / rho) - tan2);
        const double vi = nu / 120 * std::pow(cosPhi, 5) *
                          (5 - (18 * tan2) + (tan2 * tan2) + (14 * eta2) - (58 * tan2 * eta2));
        const double dLambda = (longitude - centralLongitude) * radiansPerDegree;
        return {falseEasting + (iv * dLambda) + (v * std::pow(dLambda, 3)) + (vi * std::pow(dLambda, 5)),
                i + (ii * dLambda * dLambda) + (iii * std::pow(dLambda, 4)) + (iiia * std::pow(dLambda, 6))};
    }

    /// Easting and northing in meters to latitude and longitude in degrees (C6 to C9), iterating until the northing
    /// of the latitude found is within 0.01 mm.
    std::array<double, 2> inverse(double easting, double northing) const {
        constexpr double northingTolerance = 0.01e-3;
        const double aF0 = ellipsoid.a * scale;
        double phi = ((northing - falseNorthing) / aF0) + (originLatitude * radiansPerDegree);
        double residual = northing - falseNorthing - meridianArc(phi);
        while (std::abs(residual) >= northingTolerance) {
            phi += residual / aF0;
            residual = northing - falseNorthing - meridianArc(phi);
        }
        const double tanPhi = std::tan(phi);
        const double tan2 = tanPhi * tanPhi;
        const double secPhi = 1 / std::cos(phi);
        const auto [nu, rho, eta2] = radii(phi);
        const double vii = tanPhi / (2 * rho * nu);
        const double viii = tanPhi / (24 * rho * std::pow(nu, 3)) * (5 + (3 * tan2) + eta2 - (9 * tan2 * eta2));
        const double ix = tanPhi / (720 * rho * std::pow(nu, 5)) * (61 + (90 * tan2) + (45 * tan2 * tan2));
        const double x = secPhi / nu;
        const double xi = secPhi / (6 * std::pow(nu, 3)) * ((nu / rho) + (2 * tan2));
        const double xii = secPhi / (120 * std::pow(nu, 5)) * (5 + (28 * tan2) + (24 * tan2 * tan2));
        const double xiia = secPhi / (5040 * std::pow(nu, 7)) *
                            (61 + (662 * tan2) + (1320 * tan2 * tan2) + (720 * std::pow(tan2, 3)));
        const double dE = easting - falseEasting;
        const double latitude = phi - (vii * dE * dE) + (viii * std::pow(dE, 4)) - (ix * std::pow(dE, 6));
        const double dLambda = (x * dE) - (xi * std::pow(dE, 3)) + (xii * std::pow(dE, 5)) - (xiia * std::pow(dE, 7));
        return {latitude / radiansPerDegree, centralLongitude + (dLambda / radiansPerDegree)};
    }

private:
    struct Radii {
        double nu;
        double rho;
        double eta2;
    };

    /// The radii of curvature in the prime vertical and along the meridian, scaled by F0, and their ratio less one
    /// (C2).
    Radii radii(double phi) const {
        const double e2 = ellipsoid.e2();
        const double w = 1 - (e2 * std::sin(phi) * std::sin(phi));
        const double nu = ellipsoid.a * scale / std::sqrt(w);
        const double rho = ellipsoid.a * scale * (1 - e2) / std::pow(w, 1.5);
        return {.nu = nu, .rho = rho, .eta2 = (nu / rho) - 1};
    }

    /// The meridional arc from the true origin's latitude to `phi`, scaled by F0 (C3).
    double meridianArc(double phi) const {
        const double n = (ellipsoid.a - ellipsoid.b) / (ellipsoid.a + ellipsoid.b);
        const double n2 = n * n;
        const double n3 = n2 * n;
        const double phi0 = originLatitude * radiansPerDegree;
        const double d = phi - phi0;
        const double s = phi + phi0;
        return ellipsoid.b * scale *
               (((1 + n + (5 * n2 / 4) + (5 * n3 / 4)) * d) -
                (((3 * n) + (3 * n2) + (21 * n3 / 8)) * std::sin(d) * std::cos(s)) +
                (((15 * n2 / 8) + (15 * n3 / 8)) * std::sin(2 * d) * std::cos(2 * s)) -
                ((35 * n3 / 24) * std::sin(3 * d) * std::cos(3 * s)));
    }
};

/// The National Grid's projection of OSGB36 (annex A.2).
inline constexpr TransverseMercator nationalGridProjection{.ellipsoid = airy1830,
                                                           .scale = 0.9996012717,
                                                           .originLatitude = 49,
                                                           .centralLongitude = -2,
                                                           .falseEasting = 400000,
                                                           .falseNorthing = -100000};

/// A WGS84 latitude and longitude at sea level to National Grid easting and northing.
inline std::array<double, 2> eastingNorthingFromWgs84(double latitude, double longitude) {
    const LatLngHeight osgb36 = latLngHeightFromCartesian(
        airy1830,
        etrs89ToOsgb36(cartesianFromLatLngHeight(grs80, {.latitude = latitude, .longitude = longitude, .height = 0})));
    return nationalGridProjection.forward(osgb36.latitude, osgb36.longitude);
}

/// A National Grid easting and northing to WGS84 latitude and longitude.
inline std::array<double, 2> wgs84FromEastingNorthing(double easting, double northing) {
    const auto [latitude, longitude] = nationalGridProjection.inverse(easting, northing);
    const LatLngHeight wgs84 = latLngHeightFromCartesian(
        grs80,
        etrs89ToOsgb36.reversed()(
            cartesianFromLatLngHeight(airy1830, {.latitude = latitude, .longitude = longitude, .height = 0})));
    return {wgs84.latitude, wgs84.longitude};
}

/// The side of an Ordnance Survey level 0 tile in meters. Level 0 is 5 by 6 of them from E -238375, N 1376256.
inline constexpr double level0TileSize = 229376;
/// The tile zoom of OS level 0: the registered tile 0/0/0 is the 8 by 8 square of level 0 tiles around Great Britain.
inline constexpr int level0Zoom = 3;

} // namespace national_grid

/// EPSG:27700, the British National Grid, from WGS84 through the Helmert transformation, on Ordnance Survey's tile
/// grid, where a tile's zoom is its OS level plus `national_grid::level0Zoom`.
inline CRS nationalGrid(std::string name) {
    return {.name = std::move(name),
            .forward =
                [](std::span<const std::array<double, 2>> lngLats, std::span<std::array<double, 2>> positions) {
                    for (std::size_t i = 0; i < lngLats.size(); ++i) {
                        positions[i] = national_grid::eastingNorthingFromWgs84(lngLats[i][1], lngLats[i][0]);
                    }
                },
            .inverse =
                [](std::span<const std::array<double, 2>> positions, std::span<std::array<double, 2>> lngLats) {
                    for (std::size_t i = 0; i < positions.size(); ++i) {
                        const auto [latitude, longitude] = national_grid::wgs84FromEastingNorthing(positions[i][0],
                                                                                                   positions[i][1]);
                        lngLats[i] = {longitude, latitude};
                    }
                },
            .tileMatrixSet = {.origin = {-238375, 1376256},
                              .extentAtZoom0 = national_grid::level0TileSize * (1 << national_grid::level0Zoom)}};
}

} // namespace test
} // namespace mln
