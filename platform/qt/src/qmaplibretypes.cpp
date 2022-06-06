#include "qmaplibretypes.hpp"

#include "mbgl/util/geometry.hpp"
#include "mbgl/util/traits.hpp"

// mbgl::FeatureType
static_assert(mbgl::underlying_type(QMapLibre::Feature::PointType) == mbgl::underlying_type(mbgl::FeatureType::Point), "error");
static_assert(mbgl::underlying_type(QMapLibre::Feature::LineStringType) == mbgl::underlying_type(mbgl::FeatureType::LineString), "error");
static_assert(mbgl::underlying_type(QMapLibre::Feature::PolygonType) == mbgl::underlying_type(mbgl::FeatureType::Polygon), "error");

namespace QMapLibre {

/*!
    \namespace QMapLibre
    \inmodule MapLibre Maps SDK for Qt

    Contains miscellaneous MapLibre types and utilities used throughout QMapLibreGL.
*/

/*!
    \typedef QMapLibre::Coordinate

    Alias for QPair<double, double>.
    Representation for geographical coordinates - latitude and longitude, respectively.
*/

/*!
    \typedef QMapLibre::CoordinateZoom

    Alias for QPair<Coordinate, double>.
    Used as return value in QMapLibreGL::coordinateZoomForBounds.
*/

/*!
    \typedef QMapLibre::ProjectedMeters

    Alias for QPair<double, double>.
    Representation for projected meters - northing and easting, respectively.
*/

/*!
    \typedef QMapLibre::Coordinates

    Alias for QVector<QMapLibre::Coordinate>.
    A list of QMapLibre::Coordinate objects.
*/

/*!
    \typedef QMapLibre::CoordinatesCollection

    Alias for QVector<QMapLibre::Coordinates>.
    A list of QMapLibre::Coordinates objects.
*/

/*!
    \typedef QMapLibre::CoordinatesCollections

    Alias for QVector<QMapLibre::CoordinatesCollection>.
    A list of QMapLibre::CoordinatesCollection objects.
*/

/*!
    \class QMapLibre::Feature

    \inmodule MapLibre Maps SDK for Qt

    Represents \l {https://www.mapbox.com/help/define-features/}{map features}
    via its \a type (PointType, LineStringType or PolygonType), \a geometry, \a
    properties map and \a id (optional).
*/

/*!
    \enum QMapLibre::Feature::Type

    This enum is used as basis for geometry disambiguation in QMapLibre::Feature.

    \value PointType      A point geometry type. Means a single or a collection of points.
    \value LineStringType A line string geometry type. Means a single or a collection of line strings.
    \value PolygonType    A polygon geometry type. Means a single or a collection of polygons.
*/

/*!
    \class QMapLibre::ShapeAnnotationGeometry

    \inmodule MapLibre Maps SDK for Qt

    Represents a shape annotation geometry.
*/

/*!
    \enum QMapLibre::ShapeAnnotationGeometry::Type

    This enum is used as basis for shape annotation geometry disambiguation.

    \value PolygonType         A polygon geometry type.
    \value LineStringType      A line string geometry type.
    \value MultiPolygonType    A polygon geometry collection type.
    \value MultiLineStringType A line string geometry collection type.
*/

/*!
    \class QMapLibre::SymbolAnnotation

    \inmodule MapLibre Maps SDK for Qt

    A symbol annotation comprises of its geometry and an icon identifier.
*/

/*!
    \class QMapLibre::LineAnnotation

    \inmodule MapLibre Maps SDK for Qt

    Represents a line annotation object, along with its properties.

    A line annotation comprises of its geometry and line properties such as opacity, width and color.
*/

/*!
    \class QMapLibre::FillAnnotation

    \inmodule MapLibre Maps SDK for Qt

    Represents a fill annotation object, along with its properties.

    A fill annotation comprises of its geometry and fill properties such as opacity, color and outline color.
*/

/*!
    \typedef QMapLibre::Annotation

    Alias for QVariant.
    Container that encapsulates either a symbol, a line, a fill or a style sourced annotation.
*/

/*!
    \typedef QMapLibre::AnnotationID

    Alias for quint32 representing an annotation identifier.
*/

/*!
    \typedef QMapLibre::AnnotationIDs

    Alias for QVector<quint32> representing a container of annotation identifiers.
*/

/*!
    \class QMapLibre::CameraOptions
    \inmodule MapLibre Maps SDK for Qt

    QMapLibre::CameraOptions provides camera options to the renderer.
*/

/*!
    \class QMapLibre::CustomLayerHostInterface

    Represents a host interface to be implemented for rendering custom layers.

    \warning This is used for delegating the rendering of a layer to the user of
    this API and is not officially supported. Use at your own risk.
*/

/*!
    \class QMapLibre::CustomLayerRenderParameters
    \inmodule MapLibre Maps SDK for Qt

    QMapLibre::CustomLayerRenderParameters provides the data passed on each render
    pass for a custom layer.
*/

} // namespace QMapLibre
