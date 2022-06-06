#ifndef QMAPLIBREUTILS_H
#define QMAPLIBREUTILS_H

#include "qmaplibreexport.hpp"
#include "qmaplibretypes.hpp"

// This header follows the Qt coding style: https://wiki.qt.io/Qt_Coding_Style

namespace QMapLibre {

enum NetworkMode {
    Online, // Default
    Offline,
};

Q_MAPLIBREGL_EXPORT NetworkMode networkMode();
Q_MAPLIBREGL_EXPORT void setNetworkMode(NetworkMode);

Q_MAPLIBREGL_EXPORT double metersPerPixelAtLatitude(double latitude, double zoom);
Q_MAPLIBREGL_EXPORT ProjectedMeters projectedMetersForCoordinate(const Coordinate &);
Q_MAPLIBREGL_EXPORT Coordinate coordinateForProjectedMeters(const ProjectedMeters &);

} // namespace QMapLibre

#endif // QMAPLIBRETYPES_H
