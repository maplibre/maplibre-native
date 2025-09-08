#import <Foundation/Foundation.h>

#import "MLNAnnotation.h"
#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/**
 ``MLNShape`` is an abstract class that represents a shape or annotation. Shapes
 constitute the content of a map — not only the overlays atop the map, but also
 the content that forms the base map.

 Create instances of ``MLNPointAnnotation``, ``MLNPointAnnotation``, ``MLNPointAnnotation``,
 ``MLNMultiPolyline``, ``MLNMultiPolyline``, ``MLNMultiPolyline``, or ``MLNMultiPolyline`` in
 order to use ``MLNShape``'s methods. Do not create instances of ``MLNShape``
 directly, and do not create your own subclasses of this class. The shape
 classes correspond to the
 <a href="https://tools.ietf.org/html/rfc7946#section-3.1">Geometry</a> object
 types in the GeoJSON standard, but some have nonstandard names for backwards
 compatibility.

 Although you do not create instances of this class directly, you can use its
 ``MLNShape/shapeWithData:encoding:error:`` factory method to create one of the
 concrete subclasses of ``MLNShape`` noted above from GeoJSON data. To access a
 shape’s attributes, use the corresponding ``MLNFeature`` class instead.

 You can add shapes to the map by adding them to an ``MLNShapeSource`` object.
 Configure the appearance of an ``MLNShapeSource``’s or ``MLNShapeSource``’s
 shapes collectively using a concrete instance of ``MLNVectorStyleLayer``.
 Alternatively, you can add some kinds of shapes directly to a map view as
 annotations or overlays.

 You can filter the features in a ``MLNVectorStyleLayer`` or vary their layout or
 paint attributes based on the features’ geographies. Pass an ``MLNShape`` into an
 `NSPredicate` with the format `SELF IN %@` or `%@ CONTAINS SELF` and set the
 ``MLNVectorStyleLayer/predicate`` property to that predicate, or set a layout or
 paint attribute to a similarly formatted `NSExpression`.
 */
MLN_EXPORT
@interface MLNShape : NSObject <MLNAnnotation, NSSecureCoding>

// MARK: Creating a Shape

/**
 Returns an ``MLNShape`` object initialized with the given data interpreted as a
 string containing a GeoJSON object.

 If the GeoJSON object is a geometry, the returned value is a kind of
 ``MLNShape``. If it is a feature object, the returned value is a kind of
 ``MLNShape`` that conforms to the ``MLNShape`` protocol. If it is a feature
 collection object, the returned value is an instance of
 ``MLNShapeCollectionFeature``.

 ### Example

 ```swift
 let url = mainBundle.url(forResource: "amsterdam", withExtension: "geojson")!
 let data = try! Data(contentsOf: url)
 let feature = try! MLNShape(data: data, encoding: String.Encoding.utf8.rawValue) as!
 MLNShapeCollectionFeature
 ```

 @param data String data containing GeoJSON source code.
 @param encoding The encoding used by `data`.
 @param outError Upon return, if an error has occurred, a pointer to an
    `NSError` object describing the error. Pass in `NULL` to ignore any error.
 @return An ``MLNShape`` object representation of `data`, or `nil` if `data` could
    not be parsed as valid GeoJSON source code. If `nil`, `outError` contains an
    `NSError` object describing the problem.
 */
+ (nullable MLNShape *)shapeWithData:(NSData *)data
                            encoding:(NSStringEncoding)encoding
                               error:(NSError *_Nullable *)outError;

// MARK: Accessing the Shape Attributes

/**
 The title of the shape annotation.

 The default value of this property is `nil`.

 This property is ignored when the shape is used in an ``MLNShapeSource``. To name
 a shape used in a shape source, create an ``MLNFeature`` and add an attribute to
 the ``MLNFeature/attributes`` property.
 */
@property (nonatomic, copy, nullable) NSString *title;

/**
 The subtitle of the shape annotation. The default value of this property is
 `nil`.

 This property is ignored when the shape is used in an ``MLNShapeSource``. To
 provide additional information about a shape used in a shape source, create an
 ``MLNFeature`` and add an attribute to the ``MLNFeature/attributes`` property.
 */
@property (nonatomic, copy, nullable) NSString *subtitle;

#if !TARGET_OS_IPHONE

/**
 The tooltip of the shape annotation.

 The default value of this property is `nil`.

 This property is ignored when the shape is used in an ``MLNShapeSource``.
 */
@property (nonatomic, copy, nullable) NSString *toolTip;

#endif

// MARK: Creating GeoJSON Data

/**
 Returns the GeoJSON string representation of the shape encapsulated in a data
 object.

 @param encoding The string encoding to use.
 @return A data object containing the shape’s GeoJSON string representation.
 */
- (NSData *)geoJSONDataUsingEncoding:(NSStringEncoding)encoding;

@end

NS_ASSUME_NONNULL_END
