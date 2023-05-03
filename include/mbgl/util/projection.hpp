#pragma once

#include <mbgl/util/constants.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/math/clamp.hpp>

namespace mbgl {

class ProjectedMeters {
private:
    double _northing; // Distance measured northwards.
    double _easting;  // Distance measured eastwards.

public:
    ProjectedMeters(double n_ = 0, double e_ = 0)
        : _northing(n_),
          _easting(e_) {
        if (std::isnan(_northing)) {
            throw std::domain_error("northing must not be NaN");
        }
        if (std::isnan(_easting)) {
            throw std::domain_error("easting must not be NaN");
        }
    }

    double northing() const { return _northing; }
    double easting() const { return _easting; }

    friend bool operator==(const ProjectedMeters& a, const ProjectedMeters& b) {
        return a._northing == b._northing && a._easting == b._easting;
    }

    friend bool operator!=(const ProjectedMeters& a, const ProjectedMeters& b) { return !(a == b); }
};

/// Spherical Mercator projection
/// https://github.com/openlayers/docs/blob/c7ab69156e17bedc1059d64d7a864b7868e354c1/library/spherical_mercator.rst
class Projection {
public:
    // Map pixel width at given scale.
    static double worldSize(double scale) { return scale * util::tileSize_D; }

    static double getMetersPerPixelAtLatitude(double lat, double zoom) {
        const double constrainedZoom = util::clamp(zoom, util::MIN_ZOOM, util::MAX_ZOOM);
        const double constrainedScale = std::pow(2.0, constrainedZoom);
        const double constrainedLatitude = util::clamp(lat, -util::LATITUDE_MAX, util::LATITUDE_MAX);
        return std::cos(util::deg2rad(constrainedLatitude)) * util::M2PI * util::EARTH_RADIUS_M /
               worldSize(constrainedScale);
    }

    static ProjectedMeters projectedMetersForLatLng(const LatLng& latLng) {
        const double constrainedLatitude = util::clamp(latLng.latitude(), -util::LATITUDE_MAX, util::LATITUDE_MAX);
        const double constrainedLongitude = util::clamp(latLng.longitude(), -util::LONGITUDE_MAX, util::LONGITUDE_MAX);

        const double m = 1 - 1e-15;
        const double f = util::clamp(std::sin(util::deg2rad(constrainedLatitude)), -m, m);

        const double easting = util::deg2rad(util::EARTH_RADIUS_M * constrainedLongitude);
        const double northing = 0.5 * util::EARTH_RADIUS_M * std::log((1 + f) / (1 - f));

        return {northing, easting};
    }

    static LatLng latLngForProjectedMeters(const ProjectedMeters& projectedMeters) {
        double latitude = util::rad2deg(2 * std::atan(std::exp(projectedMeters.northing() / util::EARTH_RADIUS_M)) -
                                        (M_PI / 2.0));
        double longitude = util::rad2deg(projectedMeters.easting()) / util::EARTH_RADIUS_M;

        latitude = util::clamp(latitude, -util::LATITUDE_MAX, util::LATITUDE_MAX);
        longitude = util::clamp(longitude, -util::LONGITUDE_MAX, util::LONGITUDE_MAX);

        return {latitude, longitude};
    }

    static Point<double> project(const LatLng& latLng, double scale) { return project_(latLng, worldSize(scale)); }

    /// Returns point on tile
    static Point<double> project(const LatLng& latLng, int32_t zoom) { return project_(latLng, 1 << zoom); }

    static LatLng unproject(const Point<double>& p, double scale, LatLng::WrapMode wrapMode = LatLng::Unwrapped) {
        auto p2 = p * util::DEGREES_MAX / worldSize(scale);
        return LatLng{std::atan(std::exp(util::deg2rad(util::LONGITUDE_MAX - p2.y))) * util::DEGREES_MAX / M_PI - 90.0,
                      p2.x - util::LONGITUDE_MAX,
                      wrapMode};
    }

private:
    static Point<double> project_(const LatLng& latLng, double worldSize) {
        const double latitude = util::clamp(latLng.latitude(), -util::LATITUDE_MAX, util::LATITUDE_MAX);
        return Point<double>{util::LONGITUDE_MAX + latLng.longitude(),
                             util::LONGITUDE_MAX -
                                 util::rad2deg(std::log(std::tan(M_PI / 4 + latitude * M_PI / util::DEGREES_MAX)))} *
               (worldSize / util::DEGREES_MAX);
    }
};

} // namespace mbgl
