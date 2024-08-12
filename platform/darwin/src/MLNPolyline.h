#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNMultiPoint.h"
#import "MLNOverlay.h"

#import "MLNTypes.h"

NS_ASSUME_NONNULL_BEGIN

/**
 An `MLNPolyline` object represents a shape consisting of two or more vertices,
 specified as `CLLocationCoordinate2D` instances, and the line segments that
 connect them. For example, you could use an polyline to represent a road or the
 path along which something moves.

 You can add polyline shapes to the map by adding them to an ``MLNShapeSource``
 object. Configure the appearance of an ``MLNShapeSource``’s or
 ``MLNVectorTileSource``’s polylines collectively using an ``MLNLineStyleLayer`` or
 ``MLNSymbolStyleLayer`` object. To access a polyline’s attributes, use an
 ``MLNPolylineFeature`` object.

 Alternatively, you can add a polyline overlay directly to a map view using the
 ``MLNMapView/addAnnotation:`` or ``MLNMapView/addOverlay:`` methods. Configure
 a polyline overlay’s appearance using
 ``MLNMapViewDelegate/mapView:strokeColorForShapeAnnotation:`` and
 ``MLNMapViewDelegate/mapView:lineWidthForPolylineAnnotation:``.

 The vertices are automatically connected in the order in which you provide
 them. The first and last vertices are not connected to each other, but you can
 specify the same `CLLocationCoordinate2D` as the first and last vertices in
 order to close the polyline. To fill the space within the shape, use an
 ``MLNPolygon`` object. To group multiple polylines together in one shape, use an
 ``MLNMultiPolyline`` or ``MLNShapeCollection`` object.

 To make the polyline go across the antimeridian or international date line,
 specify some longitudes less than −180 degrees or greater than 180 degrees.
 For example, a polyline that stretches from Tokyo to San Francisco would have
 coordinates of (35.68476, -220.24257) and (37.78428, -122.41310).

 ```swift
 let coordinates = [
     CLLocationCoordinate2D(latitude: 35.68476, longitude: -220.24257),
     CLLocationCoordinate2D(latitude: 37.78428, longitude: -122.41310)
 ]
 let polyline = MLNPolyline(coordinates: coordinates, count: UInt(coordinates.count))
 ```

 A polyline is known as a
 <a href="https://tools.ietf.org/html/rfc7946#section-3.1.4">LineString</a>
 geometry in GeoJSON.

 #### Related examples

 - <doc:LineOnUserTap>
 - <doc:AnimatedLineExample>
 */
MLN_EXPORT
@interface MLNPolyline : MLNMultiPoint <MLNOverlay>

/**
 Creates and returns an `MLNPolyline` object from the specified set of
 coordinates.

 @param coords The array of coordinates defining the shape. The data in this
    array is copied to the new object.
 @param count The number of items in the `coords` array.
 @return A new polyline object.
 */
+ (instancetype)polylineWithCoordinates:(const CLLocationCoordinate2D *)coords
                                  count:(NSUInteger)count;

@end

/**
 An ``MLNMultiPolyline`` object represents a shape consisting of one or more
 polylines. For example, you could use a multipolyline shape to represent both
 sides of a divided highway (dual carriageway), excluding the median (central
 reservation): each carriageway would be a distinct `MLNPolyline` object.

 You can add multipolyline shapes to the map by adding them to an
 ``MLNShapeSource`` object. Configure the appearance of an `MLNShapeSource`’s or
 ``MLNVectorTileSource``’s multipolylines collectively using an
 ``MLNLineStyleLayer`` or ``MLNSymbolStyleLayer`` object.

 You cannot add an ``MLNMultiPolyline`` object directly to a map view using
 ``MLNMapView/addAnnotation:`` or ``MLNMapView/addOverlay:``. However, you can
 add the `polylines` array’s items as overlays individually.

 A multipolyline is known as a
 <a href="https://tools.ietf.org/html/rfc7946#section-3.1.5">MultiLineString</a>
 geometry in GeoJSON.
 */
MLN_EXPORT
@interface MLNMultiPolyline : MLNShape <MLNOverlay>

/**
 An array of polygons forming the multipolyline.
 */
@property (nonatomic, copy, readonly) NSArray<MLNPolyline *> *polylines;

/**
 Creates and returns a multipolyline object consisting of the given polylines.

 @param polylines The array of polylines defining the shape.
 @return A new multipolyline object.
 */
+ (instancetype)multiPolylineWithPolylines:(NSArray<MLNPolyline *> *)polylines;

@end

NS_ASSUME_NONNULL_END
