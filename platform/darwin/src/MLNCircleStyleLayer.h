// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNFoundation.h"
#import "MLNVectorStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

/**
 Orientation of circle when map is pitched.

 Values of this type are used in the ``MLNCircleStyleLayer/circlePitchAlignment``
 property.
 */
typedef NS_ENUM(NSUInteger, MLNCirclePitchAlignment) {
    /**
     The circle is aligned to the plane of the map.
     */
    MLNCirclePitchAlignmentMap,
    /**
     The circle is aligned to the plane of the viewport.
     */
    MLNCirclePitchAlignmentViewport,
};

/**
 Controls the scaling behavior of the circle when the map is pitched.

 Values of this type are used in the ``MLNCircleStyleLayer/circleScaleAlignment``
 property.
 */
typedef NS_ENUM(NSUInteger, MLNCircleScaleAlignment) {
    /**
     Circles are scaled according to their apparent distance to the camera.
     */
    MLNCircleScaleAlignmentMap,
    /**
     Circles are not scaled.
     */
    MLNCircleScaleAlignmentViewport,
};

/**
 Controls the frame of reference for ``MLNCircleStyleLayer/circleTranslation``.

 Values of this type are used in the ``MLNCircleStyleLayer/circleTranslationAnchor``
 property.
 */
typedef NS_ENUM(NSUInteger, MLNCircleTranslationAnchor) {
    /**
     The circle is translated relative to the map.
     */
    MLNCircleTranslationAnchorMap,
    /**
     The circle is translated relative to the viewport.
     */
    MLNCircleTranslationAnchorViewport,
};

/**
 An ``MLNCircleStyleLayer`` is a style layer that renders one or more filled
 circles on the map.
 
 Use a circle style layer to configure the visual appearance of point or point
 collection features. These features can come from vector tiles loaded by an
 ``MLNCircleStyleLayer`` object, or they can be ``MLNCircleStyleLayer``,
 ``MLNCircleStyleLayer``, ``MLNCircleStyleLayer``, or ``MLNCircleStyleLayer``
 instances in an ``MLNCircleStyleLayer`` or ``MLNCircleStyleLayer`` object.
 
 A circle style layer renders circles whose radii are measured in screen units.
 To display circles on the map whose radii correspond to real-world distances,
 use many-sided regular polygons and configure their appearance using an
 ``MLNCircleStyleLayer`` object.

 You can access an existing circle style layer using the
 ``MLNStyle/layerWithIdentifier:`` method if you know its identifier;
 otherwise, find it using the ``MLNStyle/layers`` property. You can also create a
 new circle style layer and add it to the style using a method such as
 ``MLNStyle/addLayer:``.

 #### Related examples
 See the <a
 href="https://docs.mapbox.com/ios/maps/examples/dds-circle-layer/">Data-driven
 circles</a>, <a
 href="https://docs.mapbox.com/ios/maps/examples/shape-collection/">Add multiple
 shapes from a single shape source</a>, and <a
 href="https://docs.mapbox.com/ios/maps/examples/clustering/">Cluster point
 data</a> examples to learn how to add circles to your map using this style
 layer.

 ### Example

 ```swift
 ```
 */
MLN_EXPORT
@interface MLNCircleStyleLayer : MLNVectorStyleLayer

/**
 Returns a circle style layer initialized with an identifier and source.

 After initializing and configuring the style layer, add it to a map view’s
 style using the ``MLNStyle/addLayer:`` or
 ``MLNStyle/insertLayer:belowLayer:`` method.

 @param identifier A string that uniquely identifies the source in the style to
    which it is added.
 @param source The source from which to obtain the data to style. If the source
    has not yet been added to the current style, the behavior is undefined.
 @return An initialized foreground style layer.
 */
- (instancetype)initWithIdentifier:(NSString *)identifier source:(MLNSource *)source;

// MARK: - Accessing the Layout Attributes

/**
 Sorts features in ascending order based on this value. Features with a higher
 sort key will appear above features with a lower sort key.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable and/or
 feature attributes
 */
@property (nonatomic, null_resettable) NSExpression *circleSortKey;

// MARK: - Accessing the Paint Attributes

/**
 Amount to blur the circle. 1 blurs the circle such that only the centerpoint is
 full opacity.
 
 The default value of this property is an expression that evaluates to the float
 `0`. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable and/or
 feature attributes
 */
@property (nonatomic, null_resettable) NSExpression *circleBlur;

/**
 The transition affecting any changes to this layer’s `circleBlur` property.

 This property corresponds to the `circle-blur-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition circleBlurTransition;

#if TARGET_OS_IPHONE
/**
 The fill color of the circle.
 
 The default value of this property is an expression that evaluates to
 `UIColor.blackColor`. Set this property to `nil` to reset it to the default
 value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `UIColor` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable and/or
 feature attributes
 */
@property (nonatomic, null_resettable) NSExpression *circleColor;
#else
/**
 The fill color of the circle.
 
 The default value of this property is an expression that evaluates to
 `NSColor.blackColor`. Set this property to `nil` to reset it to the default
 value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `NSColor` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable and/or
 feature attributes
 */
@property (nonatomic, null_resettable) NSExpression *circleColor;
#endif

/**
 The transition affecting any changes to this layer’s `circleColor` property.

 This property corresponds to the `circle-color-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition circleColorTransition;

/**
 The opacity at which the circle will be drawn.
 
 The default value of this property is an expression that evaluates to the float
 `1`. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric values between 0 and 1 inclusive
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable and/or
 feature attributes
 */
@property (nonatomic, null_resettable) NSExpression *circleOpacity;

/**
 The transition affecting any changes to this layer’s `circleOpacity` property.

 This property corresponds to the `circle-opacity-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition circleOpacityTransition;

/**
 Orientation of circle when map is pitched.
 
 The default value of this property is an expression that evaluates to
 `viewport`. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `MLNCirclePitchAlignment` values
 * Any of the following constant string values:
   * `map`: The circle is aligned to the plane of the map.
   * `viewport`: The circle is aligned to the plane of the viewport.
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation functions to the
 `$zoomLevel` variable or applying interpolation or step functions to feature
 attributes.
 */
@property (nonatomic, null_resettable) NSExpression *circlePitchAlignment;

/**
 Circle radius.
 
 This property is measured in points.
 
 The default value of this property is an expression that evaluates to the float
 `5`. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric values no less than 0
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable and/or
 feature attributes
 */
@property (nonatomic, null_resettable) NSExpression *circleRadius;

/**
 The transition affecting any changes to this layer’s `circleRadius` property.

 This property corresponds to the `circle-radius-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition circleRadiusTransition;

/**
 Controls the scaling behavior of the circle when the map is pitched.
 
 The default value of this property is an expression that evaluates to `map`.
 Set this property to `nil` to reset it to the default value.
 
 This attribute corresponds to the <a
 href="https://maplibre.org/maplibre-style-spec/#paint-circle-pitch-scale"><code>circle-pitch-scale</code></a>
 layout property in the MapLibre Style Spec.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `MLNCircleScaleAlignment` values
 * Any of the following constant string values:
   * `map`: Circles are scaled according to their apparent distance to the
 camera.
   * `viewport`: Circles are not scaled.
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation functions to the
 `$zoomLevel` variable or applying interpolation or step functions to feature
 attributes.
 */
@property (nonatomic, null_resettable) NSExpression *circleScaleAlignment;

@property (nonatomic, null_resettable) NSExpression *circlePitchScale __attribute__((unavailable("Use circleScaleAlignment instead.")));

#if TARGET_OS_IPHONE
/**
 The stroke color of the circle.
 
 The default value of this property is an expression that evaluates to
 `UIColor.blackColor`. Set this property to `nil` to reset it to the default
 value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `UIColor` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable and/or
 feature attributes
 */
@property (nonatomic, null_resettable) NSExpression *circleStrokeColor;
#else
/**
 The stroke color of the circle.
 
 The default value of this property is an expression that evaluates to
 `NSColor.blackColor`. Set this property to `nil` to reset it to the default
 value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `NSColor` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable and/or
 feature attributes
 */
@property (nonatomic, null_resettable) NSExpression *circleStrokeColor;
#endif

/**
 The transition affecting any changes to this layer’s `circleStrokeColor` property.

 This property corresponds to the `circle-stroke-color-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition circleStrokeColorTransition;

/**
 The opacity of the circle's stroke.
 
 The default value of this property is an expression that evaluates to the float
 `1`. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric values between 0 and 1 inclusive
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable and/or
 feature attributes
 */
@property (nonatomic, null_resettable) NSExpression *circleStrokeOpacity;

/**
 The transition affecting any changes to this layer’s `circleStrokeOpacity` property.

 This property corresponds to the `circle-stroke-opacity-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition circleStrokeOpacityTransition;

/**
 The width of the circle's stroke. Strokes are placed outside of the
 `circleRadius`.
 
 This property is measured in points.
 
 The default value of this property is an expression that evaluates to the float
 `0`. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric values no less than 0
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable and/or
 feature attributes
 */
@property (nonatomic, null_resettable) NSExpression *circleStrokeWidth;

/**
 The transition affecting any changes to this layer’s `circleStrokeWidth` property.

 This property corresponds to the `circle-stroke-width-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition circleStrokeWidthTransition;

#if TARGET_OS_IPHONE
/**
 The geometry's offset.
 
 This property is measured in points.
 
 The default value of this property is an expression that evaluates to an
 `NSValue` object containing a `CGVector` struct set to 0 points rightward and 0
 points downward. Set this property to `nil` to reset it to the default value.
 
 This attribute corresponds to the <a
 href="https://maplibre.org/maplibre-style-spec/#paint-circle-translate"><code>circle-translate</code></a>
 layout property in the MapLibre Style Spec.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `CGVector` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *circleTranslation;
#else
/**
 The geometry's offset.
 
 This property is measured in points.
 
 The default value of this property is an expression that evaluates to an
 `NSValue` object containing a `CGVector` struct set to 0 points rightward and 0
 points upward. Set this property to `nil` to reset it to the default value.
 
 This attribute corresponds to the <a
 href="https://maplibre.org/maplibre-style-spec/#paint-circle-translate"><code>circle-translate</code></a>
 layout property in the MapLibre Style Spec.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `CGVector` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *circleTranslation;
#endif

/**
 The transition affecting any changes to this layer’s `circleTranslation` property.

 This property corresponds to the `circle-translate-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition circleTranslationTransition;

@property (nonatomic, null_resettable) NSExpression *circleTranslate __attribute__((unavailable("Use circleTranslation instead.")));

/**
 Controls the frame of reference for `circleTranslation`.
 
 The default value of this property is an expression that evaluates to `map`.
 Set this property to `nil` to reset it to the default value.
 
 This property is only applied to the style if `circleTranslation` is non-`nil`.
 Otherwise, it is ignored.
 
 This attribute corresponds to the <a
 href="https://maplibre.org/maplibre-style-spec/#paint-circle-translate-anchor"><code>circle-translate-anchor</code></a>
 layout property in the MapLibre Style Spec.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `MLNCircleTranslationAnchor` values
 * Any of the following constant string values:
   * `map`: The circle is translated relative to the map.
   * `viewport`: The circle is translated relative to the viewport.
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation functions to the
 `$zoomLevel` variable or applying interpolation or step functions to feature
 attributes.
 */
@property (nonatomic, null_resettable) NSExpression *circleTranslationAnchor;

@property (nonatomic, null_resettable) NSExpression *circleTranslateAnchor __attribute__((unavailable("Use circleTranslationAnchor instead.")));

@end

/**
 Methods for wrapping an enumeration value for a style layer attribute in an
 ``MLNCircleStyleLayer`` object and unwrapping its raw value.
 */
@interface NSValue (MLNCircleStyleLayerAdditions)

// MARK: Working with Circle Style Layer Attribute Values

/**
 Creates a new value object containing the given `MLNCirclePitchAlignment` enumeration.

 @param circlePitchAlignment The value for the new object.
 @return A new value object that contains the enumeration value.
 */
+ (instancetype)valueWithMLNCirclePitchAlignment:(MLNCirclePitchAlignment)circlePitchAlignment;

/**
 The ``MLNCirclePitchAlignment`` enumeration representation of the value.
 */
@property (readonly) MLNCirclePitchAlignment MLNCirclePitchAlignmentValue;

/**
 Creates a new value object containing the given `MLNCircleScaleAlignment` enumeration.

 @param circleScaleAlignment The value for the new object.
 @return A new value object that contains the enumeration value.
 */
+ (instancetype)valueWithMLNCircleScaleAlignment:(MLNCircleScaleAlignment)circleScaleAlignment;

/**
 The ``MLNCircleScaleAlignment`` enumeration representation of the value.
 */
@property (readonly) MLNCircleScaleAlignment MLNCircleScaleAlignmentValue;

/**
 Creates a new value object containing the given `MLNCircleTranslationAnchor` enumeration.

 @param circleTranslationAnchor The value for the new object.
 @return A new value object that contains the enumeration value.
 */
+ (instancetype)valueWithMLNCircleTranslationAnchor:(MLNCircleTranslationAnchor)circleTranslationAnchor;

/**
 The ``MLNCircleTranslationAnchor`` enumeration representation of the value.
 */
@property (readonly) MLNCircleTranslationAnchor MLNCircleTranslationAnchorValue;

@end

NS_ASSUME_NONNULL_END
