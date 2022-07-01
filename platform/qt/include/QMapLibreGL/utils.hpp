#ifndef QMAPLIBREGL_UTILS_H
#define QMAPLIBREGL_UTILS_H

#include "export.hpp"
#include "types.hpp"

// This header follows the Qt coding style: https://wiki.qt.io/Qt_Coding_Style

namespace QMapLibreGL {

enum NetworkMode {
    Online, // Default
    Offline,
};

Q_MAPLIBREGL_EXPORT NetworkMode networkMode();
Q_MAPLIBREGL_EXPORT void setNetworkMode(NetworkMode);

Q_MAPLIBREGL_EXPORT double metersPerPixelAtLatitude(double latitude, double zoom);
Q_MAPLIBREGL_EXPORT ProjectedMeters projectedMetersForCoordinate(const Coordinate &);
Q_MAPLIBREGL_EXPORT Coordinate coordinateForProjectedMeters(const ProjectedMeters &);

} // namespace QMapLibreGL

#endif // QMAPLIBREGL_UTILS_H
