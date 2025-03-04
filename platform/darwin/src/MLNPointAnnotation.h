#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNShape.h"

NS_ASSUME_NONNULL_BEGIN

/**
 An ``MLNPointAnnotation`` object represents a one-dimensional shape located at a
 single geographical coordinate. Depending on how it is used, an
 ``MLNPointAnnotation`` object is known as a point annotation or point shape. For
 example, you could use a point shape to represent a city at low zoom levels, an
 address at high zoom levels, or the location of a long press gesture.

 You can add point shapes to the map by adding them to an ``MLNShapeSource``
 object. Configure the appearance of an ``MLNShapeSource``’s or
 ``MLNVectorTileSource``’s point shapes collectively using an ``MLNVectorTileSource`` or
 ``MLNSymbolStyleLayer`` object.

 For more interactivity, add a selectable point annotation to a map view using
 the ``MLNMapView/addAnnotation:`` method. Alternatively, define your own model
 class that conforms to the ``MLNAnnotation`` protocol. Configure a point
 annotation’s appearance using
 ``MLNMapViewDelegate/mapView:imageForAnnotation:`` or
 ``MLNMapViewDelegate/mapView:viewForAnnotation:`` (iOS only). A point
 annotation’s ``MLNShape/title`` and ``MLNShape/title`` properties define the
 default content of the annotation’s callout (on iOS) or popover (on macOS).

 To group multiple related points together in one shape, use an
 ``MLNPointCollection`` or ``MLNPointCollection`` object. To access
 a point’s attributes, use an ``MLNPointFeature`` object.

 A point shape is known as a
 <a href="https://tools.ietf.org/html/rfc7946#section-3.1.2">Point</a> geometry
 in GeoJSON.

 #### Related examples
 TODO: Mark a place on the map with an annotation
 TODO: Mark a place on the map with an image
 TODO: Default callout usage
 Learn how to add ``MLNPointAnnotation`` objects to your map.
 */
MLN_EXPORT
@interface MLNPointAnnotation : MLNShape

/**
 The coordinate point of the shape, specified as a latitude and longitude.
 */
@property (nonatomic, assign) CLLocationCoordinate2D coordinate;

@end

NS_ASSUME_NONNULL_END
