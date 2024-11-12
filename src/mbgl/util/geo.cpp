#include <mbgl/util/geo.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/math/clamp.hpp>
#include <mbgl/util/tile_range.hpp>

#include <cmath>
#include <numbers>

using namespace std::numbers;

namespace mbgl {

namespace {

double lat_(const uint8_t z, const int64_t y) noexcept {
    const double n = pi - 2.0 * pi * y / std::pow(2.0, z);
    return util::rad2deg(std::atan(0.5 * (std::exp(n) - std::exp(-n))));
}

double lon_(const uint8_t z, const int64_t x) noexcept {
    return x / std::pow(2.0, z) * util::DEGREES_MAX - util::LONGITUDE_MAX;
}

} // end namespace

LatLng::LatLng(const CanonicalTileID& id) noexcept
    : lat(lat_(id.z, id.y)),
      lon(lon_(id.z, id.x)) {}

LatLng::LatLng(const UnwrappedTileID& id) noexcept
    : lat(lat_(id.canonical.z, id.canonical.y)),
      lon(lon_(id.canonical.z, id.canonical.x) + id.wrap * util::DEGREES_MAX) {}

LatLngBounds::LatLngBounds(const CanonicalTileID& id) noexcept
    : sw({lat_(id.z, id.y + 1), lon_(id.z, id.x)}),
      ne({lat_(id.z, id.y), lon_(id.z, id.x + 1)}) {}

bool LatLngBounds::contains(const CanonicalTileID& tileID) const noexcept {
    return util::TileRange::fromLatLngBounds(*this, tileID.z).contains(tileID);
}

bool LatLngBounds::contains(const LatLng& point, LatLng::WrapMode wrap /*= LatLng::Unwrapped*/) const noexcept {
    return containsLatitude(point.latitude()) && containsLongitude(point.longitude(), wrap);
}

bool LatLngBounds::contains(const LatLngBounds& area, LatLng::WrapMode wrap /*= LatLng::Unwrapped*/) const noexcept {
    bool containsAreaLatitude = area.north() <= north() && area.south() >= south();
    if (!containsAreaLatitude) {
        return false;
    }

    bool containsUnwrapped = area.east() <= east() && area.west() >= west();
    if (containsUnwrapped) {
        return true;
    } else if (wrap == LatLng::Wrapped) {
        const LatLngBounds wrapped(sw.wrapped(), ne.wrapped());
        const LatLngBounds other(area.sw.wrapped(), area.ne.wrapped());
        if (crossesAntimeridian() && !area.crossesAntimeridian()) {
            return (other.east() <= util::LONGITUDE_MAX && other.west() >= wrapped.west()) ||
                   (other.east() <= wrapped.east() && other.west() >= -util::LONGITUDE_MAX);
        } else {
            return other.east() <= wrapped.east() && other.west() >= wrapped.west();
        }
    }
    return false;
}

bool LatLngBounds::intersects(const LatLngBounds area, LatLng::WrapMode wrap /*= LatLng::Unwrapped*/) const noexcept {
    const bool latitudeIntersects = area.north() > south() && area.south() < north();
    if (!latitudeIntersects) {
        return false;
    }

    const bool longitudeIntersects = area.east() > west() && area.west() < east();
    if (longitudeIntersects) {
        return true;
    } else if (wrap == LatLng::Wrapped) {
        const LatLngBounds wrapped(sw.wrapped(), ne.wrapped());
        const LatLngBounds other(area.sw.wrapped(), area.ne.wrapped());
        if (crossesAntimeridian()) {
            return area.crossesAntimeridian() || other.east() > wrapped.west() || other.west() < wrapped.east();
        } else if (other.crossesAntimeridian()) {
            return other.east() > wrapped.west() || other.west() < wrapped.east();
        } else {
            return other.east() > wrapped.west() && other.west() < wrapped.east();
        }
    }
    return false;
}

LatLng LatLngBounds::constrain(const LatLng& p) const noexcept {
    if (!bounded) {
        return p;
    }

    double lat = p.latitude();
    double lng = p.longitude();

    if (!containsLatitude(lat)) {
        lat = util::clamp(lat, south(), north());
    }

    if (!containsLongitude(lng, LatLng::Unwrapped)) {
        lng = util::clamp(lng, west(), east());
    }

    return LatLng{lat, lng};
}

bool LatLngBounds::containsLatitude(double latitude) const noexcept {
    return latitude >= south() && latitude <= north();
}

bool LatLngBounds::containsLongitude(double longitude, LatLng::WrapMode wrap) const noexcept {
    if (longitude >= west() && longitude <= east()) {
        return true;
    }

    if (wrap == LatLng::Wrapped) {
        LatLngBounds wrapped(sw.wrapped(), ne.wrapped());
        longitude = LatLng(0.0, longitude).wrapped().longitude();
        if (crossesAntimeridian()) {
            return (longitude >= -util::LONGITUDE_MAX && longitude <= wrapped.east()) ||
                   (longitude >= wrapped.west() && longitude <= util::LONGITUDE_MAX);
        }

        return longitude >= wrapped.west() && longitude <= wrapped.east();
    }

    return false;
}

ScreenCoordinate EdgeInsets::getCenter(uint16_t width, uint16_t height) const noexcept {
    return {
        (width - left() - right()) / 2.0 + left(),
        (height - top() - bottom()) / 2.0 + top(),
    };
}

} // end namespace mbgl
