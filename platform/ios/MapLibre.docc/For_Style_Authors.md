# Information for Style Authors

## Designing for iOS

When designing your style, consider the context in which your application shows
the style. There are a number of considerations specific to iOS that may
not be obvious when designing your style with Maputnik. A map view
is essentially a graphical user interface element, so many of same issues in
user interface design also apply when designing a map style.

### Color

Ensure sufficient contrast in your application’s user interface when your map
style is present. Standard user interface elements such as toolbars, sidebars,
and sheets often overlap the map view with a translucent, blurred background, so
make sure the contents of these elements remain legible with the map view
underneath.
The user location annotation view, the attribution button, any buttons in
callout views, and any items in the navigation bar are influenced by your
application’s tint color, so choose a tint color that contrasts well with your
map style.
If you intend your style to be used in the dark, consider the impact that Night
Shift may have on your style’s colors.

### Typography and graphics

Choose font and icon sizes appropriate to iOS devices. iPhones and iPads have
smaller screens than the typical browser window in which you would use Mapbox
Studio, especially when multitasking is enabled. Your user’s viewing distance
may be shorter than on a desktop computer. Some of your users may use the Larger
Dynamic Type and Accessibility Text features to increase the size of all text on
the device. You can use the
[runtime styling API](Manipulating-the-style-at-runtime) to adjust your style’s
 font and icon sizes accordingly.

Design sprite images and choose font weights that look crisp on both
standard-resolution displays and Retina displays. This SDK supports the same
resolutions as iOS.
Standard-resolution displays are limited to older devices that your application
may or may not support, depending on its minimum deployment target.

Icon and text labels should be legible regardless of the map’s orientation.
By default, this SDK makes it easy for your users to rotate or tilt the map
using multitouch gestures.
If you do not intend your design to accommodate rotation and tilting, disable
these gestures using the `MLNMapView.rotateEnabled` and
`MLNMapView.pitchEnabled` properties, respectively, or the corresponding
inspectables in Interface Builder.

### Interactivity

Pay attention to whether elements of your style appear to be interactive.
A text label may look like a tappable button merely due to matching your
application’s tint color or the default blue tint color.
You can make an icon or text label interactive by installing a gesture
recognizer and performing feature querying (e.g.,
``MLNMapView/visibleFeaturesAtPoint:``) to get details about the selected
feature.

Make sure your users can easily distinguish any interactive elements from the
surrounding map, such as pins, the user location annotation view, or a route
line. Avoid relying on hover effects to indicate interactive elements. Leave
enough room between interactive elements to accommodate imprecise tapping
gestures.

For more information about user interface design, consult Apple’s
[_iOS Human Interface Guidelines_](https://developer.apple.com/ios/human-interface-guidelines/).
To learn more about designing maps for mobile devices, see [Nathaniel Slaughter's blog post](https://blog.mapbox.com/designing-maps-for-mobile-devices-32d2e49d2096) on
the subject.

## Applying your style

You set an `MLNMapView` object’s style either in code, by setting the
`MLNMapView.styleURL` property, or in Interface Builder, by setting the “Style
URL” inspectable. The URL must point to a local or remote style JSON file. The
style JSON file format is defined by the
[MapLibre Style Spec](https://maplibre.org/maplibre-style-spec/).

## Manipulating the style at runtime

The _runtime styling API_ enables you to modify every aspect of a style
dynamically as a user interacts with your application. The style itself is
represented at runtime by an ``MLNStyle`` object, which provides access to various
``MLNSource`` and ``MLNStyleLayer`` objects that represent content sources and style
layers, respectively.

To avoid conflicts with Objective-C keywords or Cocoa terminology, this SDK uses
the following terms for concepts defined in the style specification:

In the style specification | In the SDK
---------------------------|---------
bounds                     | coordinate bounds
filter                     | predicate
function type              | interpolation mode
id                         | identifier
image                      | style image
layer                      | style layer
property                   | attribute
SDF icon                   | template image
source                     | content source

## Specifying the map’s content

Each source defined by a style JSON file is represented at runtime by a content
source object that you can use to initialize new style layers. The content
source object is a member of one of the following subclasses of ``MLNSource``:

In style JSON | In the SDK
--------------|-----------
`vector`      | ``MLNVectorTileSource``
`raster`      | ``MLNRasterTileSource``
`raster-dem`  | ``MLNRasterDEMSource``
`geojson`     | ``MLNShapeSource``
`image`       | ``MLNImageSource``

`canvas` and `video` sources are not supported.

### Tile sources

Raster and vector tile sources may be defined in TileJSON configuration files.
This SDK supports the properties defined in the style specification, which are a
subset of the keys defined in version 2.1.0 of the
[TileJSON](https://github.com/mapbox/tilejson-spec/tree/master/2.1.0)
specification. As an alternative to authoring a custom TileJSON file, you may
supply various tile source options when creating a raster or vector tile source.
These options are detailed in the `MLNTileSourceOption` documentation:

In style JSON | In TileJSON   | In the SDK
--------------|---------------|-----------
`url`         | —             | `configurationURL` parameter in `-[MLNTileSource initWithIdentifier:configurationURL:]`
`tiles`       | `tiles`       | `tileURLTemplates` parameter in `-[MLNTileSource initWithIdentifier:tileURLTemplates:options:]`
`minzoom`     | `minzoom`     | `MLNTileSourceOptionMinimumZoomLevel`
`maxzoom`     | `maxzoom`     | `MLNTileSourceOptionMaximumZoomLevel`
`bounds`      | `bounds`      | `MLNTileSourceOptionCoordinateBounds`
`tileSize`    | —             | `MLNTileSourceOptionTileSize`
`attribution` | `attribution` | `MLNTileSourceOptionAttributionHTMLString` (but consider specifying `MLNTileSourceOptionAttributionInfos` instead for improved security)
`scheme`      | `scheme`      | `MLNTileSourceOptionTileCoordinateSystem`
`encoding`    | –             | `MLNTileSourceOptionDEMEncoding`

### Shape sources

Shape sources also accept various options. These options are detailed in the
`MLNShapeSourceOption` documentation:

In style JSON    | In the SDK
-----------------|-----------
`data`           | `url` parameter in `-[MLNShapeSource initWithIdentifier:URL:options:]`
`maxzoom`        | `MLNShapeSourceOptionMaximumZoomLevel`
`buffer`         | `MLNShapeSourceOptionBuffer`
`tolerance`      | `MLNShapeSourceOptionSimplificationTolerance`
`cluster`        | `MLNShapeSourceOptionClustered`
`clusterRadius`  | `MLNShapeSourceOptionClusterRadius`
`clusterMinPoints`  | `MLNShapeSourceOptionClusterMinPoints`
`clusterMaxZoom` | `MLNShapeSourceOptionMaximumZoomLevelForClustering`
`lineMetrics`    | `MLNShapeSourceOptionLineDistanceMetrics`

To create a shape source from local GeoJSON data, first
[convert the GeoJSON data into a shape](working-with-geojson-data.html#converting-geojson-data-into-shape-objects),
then use the `-[MLNShapeSource initWithIdentifier:shape:options:]` method.

### Image sources

Image sources accept a non-axis aligned quadrilateral as their geographic coordinates.
These coordinates, in `MLNCoordinateQuad`, are described in counterclockwise order,
in contrast to the clockwise order defined in the style specification.

## Configuring the map content’s appearance

Each layer defined by the style JSON file is represented at runtime by a style
layer object, which you can use to refine the map’s appearance. The style layer
object is a member of one of the following subclasses of `MLNStyleLayer`:

In style JSON | In the SDK
--------------|-----------
`background` | `MLNBackgroundStyleLayer`
`circle` | `MLNCircleStyleLayer`
`fill` | `MLNFillStyleLayer`
`fill-extrusion` | `MLNFillExtrusionStyleLayer`
`heatmap` | `MLNHeatmapStyleLayer`
`hillshade` | `MLNHillshadeStyleLayer`
`line` | `MLNLineStyleLayer`
`raster` | `MLNRasterStyleLayer`
`symbol` | `MLNSymbolStyleLayer`

You configure layout and paint attributes by setting properties on these style
layer objects. The property names generally correspond to the style JSON
properties, except for the use of camelCase instead of kebab-case. Properties
whose names differ from the style specification are listed below:

### Circle style layers

In style JSON | In Objective-C | In Swift
--------------|----------------|---------
`circle-pitch-scale` | `MLNCircleStyleLayer.circleScaleAlignment` | `MLNCircleStyleLayer.circleScaleAlignment`
`circle-translate` | `MLNCircleStyleLayer.circleTranslation` | `MLNCircleStyleLayer.circleTranslation`
`circle-translate-anchor` | `MLNCircleStyleLayer.circleTranslationAnchor` | `MLNCircleStyleLayer.circleTranslationAnchor`

### Fill style layers

In style JSON | In Objective-C | In Swift
--------------|----------------|---------
`fill-antialias` | `MLNFillStyleLayer.fillAntialiased` | `MLNFillStyleLayer.isFillAntialiased`
`fill-translate` | `MLNFillStyleLayer.fillTranslation` | `MLNFillStyleLayer.fillTranslation`
`fill-translate-anchor` | `MLNFillStyleLayer.fillTranslationAnchor` | `MLNFillStyleLayer.fillTranslationAnchor`

### Fill extrusion style layers

In style JSON | In Objective-C | In Swift
--------------|----------------|---------
`fill-extrusion-vertical-gradient` | `MLNFillExtrusionStyleLayer.fillExtrusionHasVerticalGradient` | `MLNFillExtrusionStyleLayer.fillExtrusionHasVerticalGradient`
`fill-extrusion-translate` | `MLNFillExtrusionStyleLayer.fillExtrusionTranslation` | `MLNFillExtrusionStyleLayer.fillExtrusionTranslation`
`fill-extrusion-translate-anchor` | `MLNFillExtrusionStyleLayer.fillExtrusionTranslationAnchor` | `MLNFillExtrusionStyleLayer.fillExtrusionTranslationAnchor`

### Line style layers

In style JSON | In Objective-C | In Swift
--------------|----------------|---------
`line-dasharray` | `MLNLineStyleLayer.lineDashPattern` | `MLNLineStyleLayer.lineDashPattern`
`line-translate` | `MLNLineStyleLayer.lineTranslation` | `MLNLineStyleLayer.lineTranslation`
`line-translate-anchor` | `MLNLineStyleLayer.lineTranslationAnchor` | `MLNLineStyleLayer.lineTranslationAnchor`

### Raster style layers

In style JSON | In Objective-C | In Swift
--------------|----------------|---------
`raster-brightness-max` | `MLNRasterStyleLayer.maximumRasterBrightness` | `MLNRasterStyleLayer.maximumRasterBrightness`
`raster-brightness-min` | `MLNRasterStyleLayer.minimumRasterBrightness` | `MLNRasterStyleLayer.minimumRasterBrightness`
`raster-hue-rotate` | `MLNRasterStyleLayer.rasterHueRotation` | `MLNRasterStyleLayer.rasterHueRotation`
`raster-resampling` | `MLNRasterStyleLayer.rasterResamplingMode` | `MLNRasterStyleLayer.rasterResamplingMode`

### Symbol style layers

In style JSON | In Objective-C | In Swift
--------------|----------------|---------
`icon-allow-overlap` | `MLNSymbolStyleLayer.iconAllowsOverlap` | `MLNSymbolStyleLayer.iconAllowsOverlap`
`icon-ignore-placement` | `MLNSymbolStyleLayer.iconIgnoresPlacement` | `MLNSymbolStyleLayer.iconIgnoresPlacement`
`icon-image` | `MLNSymbolStyleLayer.iconImageName` | `MLNSymbolStyleLayer.iconImageName`
`icon-optional` | `MLNSymbolStyleLayer.iconOptional` | `MLNSymbolStyleLayer.isIconOptional`
`icon-rotate` | `MLNSymbolStyleLayer.iconRotation` | `MLNSymbolStyleLayer.iconRotation`
`icon-size` | `MLNSymbolStyleLayer.iconScale` | `MLNSymbolStyleLayer.iconScale`
`icon-keep-upright` | `MLNSymbolStyleLayer.keepsIconUpright` | `MLNSymbolStyleLayer.keepsIconUpright`
`text-keep-upright` | `MLNSymbolStyleLayer.keepsTextUpright` | `MLNSymbolStyleLayer.keepsTextUpright`
`text-max-angle` | `MLNSymbolStyleLayer.maximumTextAngle` | `MLNSymbolStyleLayer.maximumTextAngle`
`text-max-width` | `MLNSymbolStyleLayer.maximumTextWidth` | `MLNSymbolStyleLayer.maximumTextWidth`
`symbol-avoid-edges` | `MLNSymbolStyleLayer.symbolAvoidsEdges` | `MLNSymbolStyleLayer.symbolAvoidsEdges`
`text-field` | `MLNSymbolStyleLayer.text` | `MLNSymbolStyleLayer.text`
`text-allow-overlap` | `MLNSymbolStyleLayer.textAllowsOverlap` | `MLNSymbolStyleLayer.textAllowsOverlap`
`text-font` | `MLNSymbolStyleLayer.textFontNames` | `MLNSymbolStyleLayer.textFontNames`
`text-size` | `MLNSymbolStyleLayer.textFontSize` | `MLNSymbolStyleLayer.textFontSize`
`text-ignore-placement` | `MLNSymbolStyleLayer.textIgnoresPlacement` | `MLNSymbolStyleLayer.textIgnoresPlacement`
`text-justify` | `MLNSymbolStyleLayer.textJustification` | `MLNSymbolStyleLayer.textJustification`
`text-optional` | `MLNSymbolStyleLayer.textOptional` | `MLNSymbolStyleLayer.isTextOptional`
`text-rotate` | `MLNSymbolStyleLayer.textRotation` | `MLNSymbolStyleLayer.textRotation`
`text-writing-mode` | `MLNSymbolStyleLayer.textWritingModes` | `MLNSymbolStyleLayer.textWritingModes`
`icon-translate` | `MLNSymbolStyleLayer.iconTranslation` | `MLNSymbolStyleLayer.iconTranslation`
`icon-translate-anchor` | `MLNSymbolStyleLayer.iconTranslationAnchor` | `MLNSymbolStyleLayer.iconTranslationAnchor`
`text-translate` | `MLNSymbolStyleLayer.textTranslation` | `MLNSymbolStyleLayer.textTranslation`
`text-translate-anchor` | `MLNSymbolStyleLayer.textTranslationAnchor` | `MLNSymbolStyleLayer.textTranslationAnchor`

## Setting attribute values

Each property representing a layout or paint attribute is set to an
`NSExpression` object. `NSExpression` objects play the same role as
[expressions in the MapLibre Style Spec](https://maplibre.org/maplibre-style-spec/expressions/),
but you create the former using a very different syntax. `NSExpression`’s format
string syntax is reminiscent of a spreadsheet formula or an expression in a
database query. See the
“[Predicates and Expressions](predicates-and-expressions.html)” guide for an
overview of the expression support in this SDK. This SDK no longer supports
style functions; use expressions instead.

### Constant values in expressions

In contrast to the JSON type that the style specification defines for each
layout or paint property, the style value object often contains a more specific
Foundation or Cocoa type. General rules for attribute types are listed below.
Pay close attention to the SDK documentation for the attribute you want to get
or set.

In style JSON | In Objective-C        | In Swift
--------------|-----------------------|---------
Color         | `UIColor` | `UIColor`
Enum          | `NSString`            | `String`
String        | `NSString`            | `String`
Boolean       | `NSNumber.boolValue`  | `NSNumber.boolValue`
Number        | `NSNumber.floatValue` | `NSNumber.floatValue`
Array (`-dasharray`) | `NSArray<NSNumber>` | `[Float]`
Array (`-font`) | `NSArray<NSString>` | `[String]`
Array (`-offset`, `-translate`) | `NSValue.CGVectorValue` | `NSValue.cgVectorValue`
Array (`-padding`) | `NSValue.UIEdgeInsetsValue` | `NSValue.uiEdgeInsetsValue`

For padding attributes, note that the arguments to
`UIEdgeInsetsMake()` in Objective-C and `UIEdgeInsets(top:left:bottom:right:)`
in Swift
are specified in counterclockwise order, in contrast to the clockwise order
defined by the style specification

## Filtering sources

You can filter a shape or vector tile source by setting the
`MLNVectorStyleLayer.predicate` property to an `NSPredicate` object. Below is a
table of style JSON operators and the corresponding operators used in the
predicate format string:

In style JSON             | In the format string
--------------------------|---------------------
`["has", key]`            | `key != nil`
`["!has", key]`           | `key == nil`
`["==", key, value]`      | `key == value`
`["!=", key, value]`      | `key != value`
`[">", key, value]`       | `key > value`
`[">=", key, value]`      | `key >= value`
`["<", key, value]`       | `key < value`
`["<=", key, value]`      | `key <= value`
`["in", key, v0, …, vn]`  | `key IN {v0, …, vn}`
`["!in", key, v0, …, vn]` | `NOT key IN {v0, …, vn}`
`["all", f0, …, fn]`      | `p0 AND … AND pn`
`["any", f0, …, fn]`      | `p0 OR … OR pn`
`["none", f0, …, fn]`     | `NOT (p0 OR … OR pn)`

## Specifying the text format

The following format attributes are defined as `NSString` constans that you
can use to update the formatting of `MLNSymbolStyleLayer.text` property.

In style JSON | In Objective-C        | In Swift
--------------|-----------------------|---------
`text-font`      | `MLNFontNamesAttribute` | `.fontNamesAttribute`
`font-scale`      | `MLNFontScaleAttribute` | `.fontScaleAttribute`
`text-color`  | `MLNFontColorAttribute` | `.fontColorAttribute`

See <doc:Predicates_and_Expressions> for
a full description of the supported operators and operand types.
