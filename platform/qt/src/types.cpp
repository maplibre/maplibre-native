#include <QMapLibreGL/Types>

#include "mbgl/util/geometry.hpp"
#include "mbgl/util/traits.hpp"

// mbgl::FeatureType
static_assert(mbgl::underlying_type(QMapLibreGL::Feature::PointType) == mbgl::underlying_type(mbgl::FeatureType::Point), "error");
static_assert(mbgl::underlying_type(QMapLibreGL::Feature::LineStringType) == mbgl::underlying_type(mbgl::FeatureType::LineString), "error");
static_assert(mbgl::underlying_type(QMapLibreGL::Feature::PolygonType) == mbgl::underlying_type(mbgl::FeatureType::Polygon), "error");

namespace QMapLibreGL {

/*!
    \namespace QMapLibreGL
    \inmodule MapLibre Maps SDK for Qt

    Contains miscellaneous MapLibre types and utilities used throughout QMapLibreGLGL.
*/

/*!
    \typedef QMapLibreGL::Coordinate

    Alias for QPair<double, double>.
    Representation for geographical coordinates - latitude and longitude, respectively.
*/

/*!
    \typedef QMapLibreGL::CoordinateZoom

    Alias for QPair<Coordinate, double>.
    Used as return value in QMapLibreGLGL::coordinateZoomForBounds.
*/

/*!
    \typedef QMapLibreGL::ProjectedMeters

    Alias for QPair<double, double>.
    Representation for projected meters - northing and easting, respectively.
*/

/*!
    \typedef QMapLibreGL::Coordinates

    Alias for QVector<QMapLibreGL::Coordinate>.
    A list of QMapLibreGL::Coordinate objects.
*/

/*!
    \typedef QMapLibreGL::CoordinatesCollection

    Alias for QVector<QMapLibreGL::Coordinates>.
    A list of QMapLibreGL::Coordinates objects.
*/

/*!
    \typedef QMapLibreGL::CoordinatesCollections

    Alias for QVector<QMapLibreGL::CoordinatesCollection>.
    A list of QMapLibreGL::CoordinatesCollection objects.
*/

/*!
    \class QMapLibreGL::Feature

    \inmodule MapLibre Maps SDK for Qt

    Represents \l {https://www.mapbox.com/help/define-features/}{map features}
    via its \a type (PointType, LineStringType or PolygonType), \a geometry, \a
    properties map and \a id (optional).
*/

/*!
    \enum QMapLibreGL::Feature::Type

    This enum is used as basis for geometry disambiguation in QMapLibreGL::Feature.

    \value PointType      A point geometry type. Means a single or a collection of points.
    \value LineStringType A line string geometry type. Means a single or a collection of line strings.
    \value PolygonType    A polygon geometry type. Means a single or a collection of polygons.
*/

/*!
    \class QMapLibreGL::ShapeAnnotationGeometry

    \inmodule MapLibre Maps SDK for Qt

    Represents a shape annotation geometry.
*/

/*!
    \enum QMapLibreGL::ShapeAnnotationGeometry::Type

    This enum is used as basis for shape annotation geometry disambiguation.

    \value PolygonType         A polygon geometry type.
    \value LineStringType      A line string geometry type.
    \value MultiPolygonType    A polygon geometry collection type.
    \value MultiLineStringType A line string geometry collection type.
*/

/*!
    \class QMapLibreGL::SymbolAnnotation

    \inmodule MapLibre Maps SDK for Qt

    A symbol annotation comprises of its geometry and an icon identifier.
*/

/*!
    \class QMapLibreGL::LineAnnotation

    \inmodule MapLibre Maps SDK for Qt

    Represents a line annotation object, along with its properties.

    A line annotation comprises of its geometry and line properties such as opacity, width and color.
*/

/*!
    \class QMapLibreGL::FillAnnotation

    \inmodule MapLibre Maps SDK for Qt

    Represents a fill annotation object, along with its properties.

    A fill annotation comprises of its geometry and fill properties such as opacity, color and outline color.
*/

/*!
    \typedef QMapLibreGL::Annotation

    Alias for QVariant.
    Container that encapsulates either a symbol, a line, a fill or a style sourced annotation.
*/

/*!
    \typedef QMapLibreGL::AnnotationID

    Alias for quint32 representing an annotation identifier.
*/

/*!
    \typedef QMapLibreGL::AnnotationIDs

    Alias for QVector<quint32> representing a container of annotation identifiers.
*/

/*!
    \class QMapLibreGL::CameraOptions
    \inmodule MapLibre Maps SDK for Qt

    QMapLibreGL::CameraOptions provides camera options to the renderer.
*/

/*!
    \class QMapLibreGL::CustomLayerHostInterface

    Represents a host interface to be implemented for rendering custom layers.

    \warning This is used for delegating the rendering of a layer to the user of
    this API and is not officially supported. Use at your own risk.
*/

/*!
    \class QMapLibreGL::CustomLayerRenderParameters
    \inmodule MapLibre Maps SDK for Qt

    QMapLibreGL::CustomLayerRenderParameters provides the data passed on each render
    pass for a custom layer.
*/

} // namespace QMapLibreGL
