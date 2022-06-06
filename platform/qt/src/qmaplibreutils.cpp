#include "qmaplibreutils.hpp"

#include <mbgl/storage/network_status.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/projection.hpp>

#include <QOpenGLContext>

// mbgl::NetworkStatus::Status
static_assert(mbgl::underlying_type(QMapLibre::Online) == mbgl::underlying_type(mbgl::NetworkStatus::Status::Online), "error");
static_assert(mbgl::underlying_type(QMapLibre::Offline) == mbgl::underlying_type(mbgl::NetworkStatus::Status::Offline), "error");

namespace QMapLibre {

/*!
    \enum QMapLibre::NetworkMode

    This enum represents whether server requests can be performed via network.

    \value Online  Server network requests are accessible.
    \value Offline Only requests to the local cache are accessible.
*/

/*!
    \fn QMapLibre::NetworkMode QMapLibre::networkMode()

    Returns the current QMapLibre::NetworkMode.
*/
NetworkMode networkMode()
{
    return static_cast<NetworkMode>(mbgl::NetworkStatus::Get());
}

/*!
    \fn void QMapLibre::setNetworkMode(QMapLibre::NetworkMode mode)

    Forwards the network status \a mode to MapLibre GL Native engine.

    File source requests uses the available network when \a mode is set to \b
    Online, otherwise scoped to the local cache.
*/
void setNetworkMode(NetworkMode mode)
{
    mbgl::NetworkStatus::Set(static_cast<mbgl::NetworkStatus::Status>(mode));
}

/*!
    Returns the amount of meters per pixel from a given \a latitude and \a zoom.
*/
double metersPerPixelAtLatitude(double latitude, double zoom)
{
    return mbgl::Projection::getMetersPerPixelAtLatitude(latitude, zoom);
}

/*!
    Return the projected meters for a given \a coordinate object.
*/
ProjectedMeters projectedMetersForCoordinate(const Coordinate &coordinate)
{
    auto projectedMeters = mbgl::Projection::projectedMetersForLatLng(mbgl::LatLng { coordinate.first, coordinate.second });
    return QMapLibre::ProjectedMeters(projectedMeters.northing(), projectedMeters.easting());
}

/*!
    Returns the coordinate for a given \a projectedMeters object.
*/
Coordinate coordinateForProjectedMeters(const ProjectedMeters &projectedMeters)
{
    auto latLng = mbgl::Projection::latLngForProjectedMeters(mbgl::ProjectedMeters { projectedMeters.first, projectedMeters.second });
    return QMapLibre::Coordinate(latLng.latitude(), latLng.longitude());
}

} // namespace QMapLibre
