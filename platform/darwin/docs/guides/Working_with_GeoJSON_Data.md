# Working with GeoJSON Data

This SDK offers several ways to work with [GeoJSON](http://geojson.org/) files.
GeoJSON is a standard file format for representing geographic data.

## Adding a GeoJSON file to the map

You can use
[Mapbox Studio’s Datasets editor](https://www.mapbox.com/studio/datasets/) to
upload a GeoJSON file and include it in your custom map style. The GeoJSON data
will be hosted on Mapbox servers. When a user loads your style, the SDK
automatically loads the GeoJSON data for display.

Alternatively, if you need to host the GeoJSON file elsewhere or bundle it with
your application, you can use a GeoJSON file as the basis of an `MLNShapeSource`
object. Pass the file’s URL into the
`-[MLNShapeSource initWithIdentifier:URL:options:]` initializer and add the
shape source to the map using the `-[MLNStyle addSource:]` method. The URL may
be a local file URL, an HTTP URL, or an HTTPS URL.

Once you’ve added the GeoJSON file to the map via an `MLNShapeSource` object,
you can configure the appearance of its data and control what data is visible
using `MLNStyleLayer` objects. You can also
[access the data programmatically](#extracting-geojson-data-from-the-map).

## Converting GeoJSON data into shape objects

If you have GeoJSON data in the form of source code (also known as “GeoJSON
text”), you can convert it into an `MLNShape`, `MLNFeature`, or
`MLNShapeCollectionFeature` object that the `MLNShapeSource` class understands
natively. First, create an `NSData` object out of the source code string or file
contents, then pass that data object into the
`+[MLNShape shapeWithData:encoding:error:]` method. Finally, you can pass the
resulting shape or feature object into the
`-[MLNShapeSource initWithIdentifier:shape:options:]` initializer and add it to
the map, or you can use the object and its properties to power non-map-related
functionality in your application.

To include multiple shapes in the source, create and pass an `MLNShapeCollection` or
 `MLNShapeCollectionFeature` object to 
 `-[MLNShapeSource initWithIdentifier:shape:options:]`. Alternatively, use the
 `-[MLNShapeSource initWithIdentifier:features:options:]` or 
 `-[MLNShapeSource initWithIdentifier:shapes:options:]` method to create a shape source 
 with an array. `-[MLNShapeSource initWithIdentifier:features:options:]` accepts only `MLNFeature`
 instances, such as `MLNPointFeature` objects, whose attributes you can use when
 applying a predicate to `MLNVectorStyleLayer` or configuring a style layer’s
 appearance.

## Extracting GeoJSON data from the map

Any `MLNShape`, `MLNFeature`, or `MLNShapeCollectionFeature` object has an
`-[MLNShape geoJSONDataUsingEncoding:]` method that you can use to create a
GeoJSON source code representation of the object. You can extract a feature
object from the map using a method such as
`-[MLNMapView visibleFeaturesAtPoint:]`.

## About GeoJSON deserialization

The process of converting GeoJSON text into `MLNShape`, `MLNFeature`, or
`MLNShapeCollectionFeature` objects is known as “GeoJSON deserialization”.
GeoJSON geometries, features, and feature collections are known in this SDK as
shapes, features, and shape collection features, respectively.

Each GeoJSON object type corresponds to a type provided by either this SDK or
the Core Location framework:

GeoJSON object type | SDK type
--------------------|---------
`Position` (longitude, latitude) | `CLLocationCoordinate2D` (latitude, longitude)
`Point`             | `MLNPointAnnotation`
`MultiPoint`        | `MLNPointCollection`
`LineString`        | `MLNPolyline`
`MultiLineString`   | `MLNMultiPolyline`
`Polygon`           | `MLNPolygon`
Linear ring         | `MLNPolygon.coordinates`, `MLNPolygon.interiorPolygons`
`MultiPolygon`      | `MLNMultiPolygon`
`GeometryCollection` | `MLNShapeCollection`
`Feature`           | `MLNFeature`
`FeatureCollection` | `MLNShapeCollectionFeature`

A `Feature` object in GeoJSON corresponds to an instance of an `MLNShape`
subclass conforming to the `MLNFeature` protocol. There is a distinct
`MLNFeature`-conforming class for each type of geometry that a GeoJSON feature
can contain. This allows features to be used as raw shapes where convenient. For
example, some features can be added to a map view as annotations. Note that
identifiers and attributes will not be available for feature querying when a
feature is used as an annotation.

In contrast to the GeoJSON standard, it is possible for `MLNShape` subclasses
other than `MLNPointAnnotation` to straddle the antimeridian.

The following GeoJSON data types correspond straightforwardly to Foundation data
types when they occur as feature identifiers or property values:

GeoJSON data type  | Objective-C representation | Swift representation
-------------------|----------------------------|---------------------
`null`             | `NSNull`                   | `NSNull`
`true`, `false`    | `NSNumber.boolValue`       | `Bool`
Integer            | `NSNumber.unsignedLongLongValue`, `NSNumber.longLongValue` | `UInt64`, `Int64`
Floating-point number | `NSNumber.doubleValue`  | `Double`
String             | `NSString`                 | `String`
