#pragma once

#include <mbgl/math/clamp.hpp>
#include <mbgl/math/wrap.hpp>
#include <mbgl/util/constants.hpp>

#include <mapbox/geometry/point.hpp>
#include <mapbox/geometry/point_arithmetic.hpp>
#include <mapbox/geometry/line_string.hpp>
#include <mapbox/geometry/box.hpp>

#include <cmath>
#include <stdexcept>

namespace mbgl {

class CanonicalTileID;
class UnwrappedTileID;

using ScreenCoordinate = mapbox::geometry::point<double>;
using ScreenLineString = mapbox::geometry::line_string<double>;
using ScreenBox = mapbox::geometry::box<double>;

class LatLng {
private:
    double lat;
    double lon;

public:
    enum WrapMode : bool {
        Unwrapped,
        Wrapped
    };

    LatLng(double lat_ = 0, double lon_ = 0, WrapMode mode = Unwrapped)
        : lat(lat_),
          lon(lon_) {
        if (std::isnan(lat)) {
            throw std::domain_error("latitude must not be NaN");
        }
        if (std::isnan(lon)) {
            throw std::domain_error("longitude must not be NaN");
        }
        if (std::abs(lat) > 90.0) {
            throw std::domain_error("latitude must be between -90 and 90");
        }
        if (!std::isfinite(lon)) {
            throw std::domain_error("longitude must not be infinite");
        }
        if (mode == Wrapped) {
            wrap();
        }
    }

    double latitude() const noexcept { return lat; }
    double longitude() const noexcept { return lon; }

    LatLng wrapped() const noexcept { return {lat, lon, Wrapped}; }

    void wrap() noexcept { lon = util::wrap(lon, -util::LONGITUDE_MAX, util::LONGITUDE_MAX); }

    // If the distance from start to end longitudes is between half and full
    // world, unwrap the start longitude to ensure the shortest path is taken.
    void unwrapForShortestPath(const LatLng& end) noexcept {
        const double delta = std::abs(end.lon - lon);
        if (delta <= util::LONGITUDE_MAX || delta >= util::DEGREES_MAX) return;
        if (lon > 0 && end.lon < 0)
            lon -= util::DEGREES_MAX;
        else if (lon < 0 && end.lon > 0)
            lon += util::DEGREES_MAX;
    }

    // Constructs a LatLng object with the top left position of the specified tile.
    LatLng(const CanonicalTileID& id) noexcept;
    LatLng(const UnwrappedTileID& id) noexcept;

    friend bool operator==(const LatLng& a, const LatLng& b) noexcept { return a.lat == b.lat && a.lon == b.lon; }

    friend bool operator!=(const LatLng& a, const LatLng& b) noexcept { return !(a == b); }
};

class LatLngBounds {
public:
    // Return a bounds covering the entire (unwrapped) world.
    static LatLngBounds world() noexcept { return {{-90, -180}, {90, 180}}; }

    // Return the bounds consisting of the single point.
    static LatLngBounds singleton(const LatLng& a) { return {a, a}; }

    // Return the convex hull of two points; the smallest bounds that contains both.
    static LatLngBounds hull(const LatLng& a, const LatLng& b) noexcept {
        LatLngBounds bounds(a, a);
        bounds.extend(b);
        return bounds;
    }

    // Return a bounds that may serve as the identity element for the extend operation.
    static LatLngBounds empty() noexcept {
        LatLngBounds bounds = world();
        std::swap(bounds.sw, bounds.ne);
        return bounds;
    }

    /// Construct an infinite bound, a bound for which the constrain method
    /// returns its input unmodified.
    ///
    /// Note: this is different than LatLngBounds::world() since arbitrary
    /// unwrapped coordinates are also inside the bounds.
    LatLngBounds() noexcept
        : sw({-90, -180}),
          ne({90, 180}),
          bounded(false) {}

    // Constructs a LatLngBounds object with the tile's exact boundaries.
    LatLngBounds(const CanonicalTileID&) noexcept;

    constexpr bool valid() const noexcept {
        return (sw.latitude() <= ne.latitude()) && (sw.longitude() <= ne.longitude());
    }

    constexpr double south() const noexcept { return sw.latitude(); }
    constexpr double west() const noexcept { return sw.longitude(); }
    constexpr double north() const noexcept { return ne.latitude(); }
    constexpr double east() const noexcept { return ne.longitude(); }

    LatLng southwest() const noexcept { return sw; }
    LatLng northeast() const noexcept { return ne; }
    LatLng southeast() const noexcept { return {south(), east()}; }
    LatLng northwest() const noexcept { return {north(), west()}; }

    LatLng center() const noexcept {
        return {(sw.latitude() + ne.latitude()) / 2, (sw.longitude() + ne.longitude()) / 2};
    }

    LatLng constrain(const LatLng& p) const noexcept;

    void extend(const LatLng& point) noexcept {
        sw = LatLng(std::min(point.latitude(), sw.latitude()), std::min(point.longitude(), sw.longitude()));
        ne = LatLng(std::max(point.latitude(), ne.latitude()), std::max(point.longitude(), ne.longitude()));
    }

    void extend(const LatLngBounds& bounds) noexcept {
        extend(bounds.sw);
        extend(bounds.ne);
    }

    constexpr bool isEmpty() const noexcept { return sw.latitude() > ne.latitude() || sw.longitude() > ne.longitude(); }

    constexpr bool crossesAntimeridian() const noexcept {
        return (sw.wrapped().longitude() > ne.wrapped().longitude());
    }

    bool contains(const CanonicalTileID& tileID) const noexcept;
    bool contains(const LatLng& point, LatLng::WrapMode wrap = LatLng::Unwrapped) const noexcept;
    bool contains(const LatLngBounds& area, LatLng::WrapMode wrap = LatLng::Unwrapped) const noexcept;

    bool intersects(LatLngBounds area, LatLng::WrapMode wrap = LatLng::Unwrapped) const noexcept;

private:
    LatLng sw;
    LatLng ne;
    bool bounded = true;

    LatLngBounds(LatLng sw_, LatLng ne_) noexcept
        : sw(sw_),
          ne(ne_) {}

    bool containsLatitude(double latitude) const noexcept;
    bool containsLongitude(double longitude, LatLng::WrapMode wrap) const noexcept;

    friend constexpr bool operator==(const LatLngBounds& a, const LatLngBounds& b) {
        return (!a.bounded && !b.bounded) || (a.bounded && b.bounded && a.sw == b.sw && a.ne == b.ne);
    }

    friend constexpr bool operator!=(const LatLngBounds& a, const LatLngBounds& b) { return !(a == b); }
};

// Determines the orientation of the map.
enum class NorthOrientation : uint8_t {
    Upwards, // Default
    Rightwards,
    Downwards,
    Leftwards,
};

/// The distance on each side between a rectangle and a rectangle within.
class EdgeInsets {
private:
    double _top;    // Number of pixels inset from the top edge.
    double _left;   // Number of pixels inset from the left edge.
    double _bottom; // Number of pixels inset from the bottom edge.
    double _right;  // Number of pixels inset from the right edge.

public:
    EdgeInsets(double t_ = 0, double l_ = 0, double b_ = 0, double r_ = 0)
        : _top(t_),
          _left(l_),
          _bottom(b_),
          _right(r_) {
        if (std::isnan(_top)) {
            throw std::domain_error("top must not be NaN");
        }
        if (std::isnan(_left)) {
            throw std::domain_error("left must not be NaN");
        }
        if (std::isnan(_bottom)) {
            throw std::domain_error("bottom must not be NaN");
        }
        if (std::isnan(_right)) {
            throw std::domain_error("right must not be NaN");
        }
    }

    double top() const noexcept { return _top; }
    double left() const noexcept { return _left; }
    double bottom() const noexcept { return _bottom; }
    double right() const noexcept { return _right; }

    bool isFlush() const noexcept { return _top == 0 && _left == 0 && _bottom == 0 && _right == 0; }

    void operator+=(const EdgeInsets& o) {
        _top += o._top;
        _left += o._left;
        _bottom += o._bottom;
        _right += o._right;
    }

    EdgeInsets operator+(const EdgeInsets& o) const noexcept {
        return {
            _top + o._top,
            _left + o._left,
            _bottom + o._bottom,
            _right + o._right,
        };
    }

    ScreenCoordinate getCenter(uint16_t width, uint16_t height) const noexcept;

    friend bool operator==(const EdgeInsets& a, const EdgeInsets& b) noexcept {
        return a._top == b._top && a._left == b._left && a._bottom == b._bottom && a._right == b._right;
    }

    friend bool operator!=(const EdgeInsets& a, const EdgeInsets& b) noexcept { return !(a == b); }
};

struct LatLngAltitude {
    LatLng location = {0.0, 0.0};
    double altitude = 0.0;
};

} // namespace mbgl
