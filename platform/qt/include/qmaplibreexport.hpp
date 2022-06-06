#ifndef QMAPLIBREEXPORT_H
#define QMAPLIBREEXPORT_H

// This header follows the Qt coding style: https://wiki.qt.io/Qt_Coding_Style

#if !defined(QT_MAPLIBREGL_STATIC)
#  if defined(QT_BUILD_MAPLIBREGL_LIB)
#    define Q_MAPLIBREGL_EXPORT Q_DECL_EXPORT
#  else
#    define Q_MAPLIBREGL_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_MAPLIBREGL_EXPORT
#endif

#endif // QMAPLIBREEXPORT_H
