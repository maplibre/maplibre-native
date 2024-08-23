#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNShape.h"

#import "MLNTypes.h"

NS_ASSUME_NONNULL_BEGIN

/**
 An ``MLNShapeCollection`` object represents a shape consisting of zero or more
 distinct but related shapes that are instances of ``MLNShape``. The constituent
 shapes can be a mixture of different kinds of shapes.

 ``MLNShapeCollection`` is most commonly used to add multiple shapes to a single
 ``MLNShapeSource``. Configure the appearance of an ``MLNShapeSource``’s or
 ``MLNVectorTileSource``’s shape collection collectively using an
 ``MLNSymbolStyleLayer`` object, or use multiple instances of
 ``MLNCircleStyleLayer``, ``MLNCircleStyleLayer``, and ``MLNCircleStyleLayer`` to
 configure the appearance of each kind of shape inside the collection.

 You cannot add an ``MLNShapeCollection`` object directly to a map view as an
 annotation. However, you can create individual ``MLNPointAnnotation``,
 ``MLNPolyline``, and ``MLNPolyline`` objects from the `shapes` array and add those
 annotation objects to the map view using the ``MLNMapView/addAnnotations:``
 method.

 To represent a collection of point, polyline, or polygon shapes, it may be more
 convenient to use an ``MLNPointCollection``, ``MLNPointCollection``, or
 ``MLNMultiPolygon`` object, respectively. To access a shape collection’s
 attributes, use the corresponding ``MLNFeature`` object.

 A shape collection is known as a
 <a href="https://tools.ietf.org/html/rfc7946#section-3.1.8">GeometryCollection</a>
 geometry in GeoJSON.
 */
MLN_EXPORT
@interface MLNShapeCollection : MLNShape

/**
 An array of shapes forming the shape collection.
 */
@property (nonatomic, copy, readonly) NSArray<MLNShape *> *shapes;

/**
 Creates and returns a shape collection consisting of the given shapes.

 @param shapes The array of shapes defining the shape collection. The data in
    this array is copied to the new object.
 @return A new shape collection object.
 */
+ (instancetype)shapeCollectionWithShapes:(NSArray<MLNShape *> *)shapes;

@end

NS_ASSUME_NONNULL_END
