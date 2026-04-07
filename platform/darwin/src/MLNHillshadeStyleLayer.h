// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNFoundation.h"
#import "MLNForegroundStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

/**
 Direction of light source when map is rotated.

 Values of this type are used in the ``MLNHillshadeStyleLayer/hillshadeIlluminationAnchor``
 property.
 */
typedef NS_ENUM(NSUInteger, MLNHillshadeIlluminationAnchor) {
    /**
     The hillshade illumination is relative to the north direction.
     */
    MLNHillshadeIlluminationAnchorMap,
    /**
     The hillshade illumination is relative to the top of the viewport.
     */
    MLNHillshadeIlluminationAnchorViewport,
};

/**
 The hillshade algorithm to use, one of `MLNHillshadeMethodStandard`,
 `MLNHillshadeMethodBasic`, `MLNHillshadeMethodCombined`,
 `MLNHillshadeMethodIgor`, or `MLNHillshadeMethodMultidirectional`.
 ![image](assets/hillshade_methods.png)

 Values of this type are used in the ``MLNHillshadeStyleLayer/hillshadeMethod``
 property.
 */
typedef NS_ENUM(NSUInteger, MLNHillshadeMethod) {
    /**
     The legacy hillshade method.
     */
    MLNHillshadeMethodStandard,
    /**
     Basic hillshade. Uses a simple physics model where the reflected light
     intensity is proportional to the cosine of the angle between the incident
     light and the surface normal. Similar to GDAL's `gdaldem` default
     algorithm.
     */
    MLNHillshadeMethodBasic,
    /**
     Hillshade algorithm whose intensity scales with slope. Similar to GDAL's
     `gdaldem` with `Combined` option.
     */
    MLNHillshadeMethodCombined,
    /**
     Hillshade algorithm which tries to minimize effects on other map features
     beneath. Similar to GDAL's `gdaldem` with `Igor` option.
     */
    MLNHillshadeMethodIgor,
    /**
     Hillshade with multiple illumination directions. Uses the basic hillshade
     model with multiple independent light sources.
     */
    MLNHillshadeMethodMultidirectional,
};

/**
 An ``MLNHillshadeStyleLayer`` is a style layer that renders raster <a
 href="https://en.wikipedia.org/wiki/Digital_elevation_model">digital elevation
 model</a> (DEM) tiles on the map.
 
 Use a hillshade style layer to configure the color parameters of raster tiles
 loaded by an ``MLNHillshadeStyleLayer`` object. For example, you could use a
 hillshade style layer to render <a
 href="https://docs.mapbox.com/help/troubleshooting/access-elevation-data/#mapbox-terrain-rgb">Mapbox
 Terrain-RGB</a> data.
 
 To display posterized hillshading based on vector shapes, as with the <a
 href="https://www.mapbox.com/vector-tiles/mapbox-terrain/">Mapbox Terrain</a>
 source, use an ``MLNHillshadeStyleLayer`` object in conjunction with several
 ``MLNHillshadeStyleLayer`` objects.

 You can access an existing hillshade style layer using the
 ``MLNStyle/layerWithIdentifier:`` method if you know its identifier;
 otherwise, find it using the ``MLNStyle/layers`` property. You can also create a
 new hillshade style layer and add it to the style using a method such as
 ``MLNStyle/addLayer:``.

 ### Example

 ```swift
 ```
 */
MLN_EXPORT
@interface MLNHillshadeStyleLayer : MLNForegroundStyleLayer

/**
 Returns a hillshade style layer initialized with an identifier and source.

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

// MARK: - Accessing the Paint Attributes

#if TARGET_OS_IPHONE
/**
 The shading color used to accentuate rugged terrain like sharp cliffs and
 gorges.
 
 The default value of this property is an expression that evaluates to
 `UIColor.blackColor`. Set this property to `nil` to reset it to the default
 value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `UIColor` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeAccentColor;
#else
/**
 The shading color used to accentuate rugged terrain like sharp cliffs and
 gorges.
 
 The default value of this property is an expression that evaluates to
 `NSColor.blackColor`. Set this property to `nil` to reset it to the default
 value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `NSColor` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeAccentColor;
#endif

/**
 The transition affecting any changes to this layer’s `hillshadeAccentColor` property.

 This property corresponds to the `hillshade-accent-color-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition hillshadeAccentColorTransition;

/**
 Intensity of the hillshade
 
 The default value of this property is an expression that evaluates to the float
 `0.5`. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric values between 0 and 1 inclusive
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeExaggeration;

/**
 The transition affecting any changes to this layer’s `hillshadeExaggeration` property.

 This property corresponds to the `hillshade-exaggeration-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition hillshadeExaggerationTransition;

#if TARGET_OS_IPHONE
/**
 The shading color of areas that faces towards the light source(s). Only when
 `hillshadeMethod` is set to `MLNHillshadeMethodMultidirectional` can you
 specify multiple light sources.
 
 The default value of this property is an expression that evaluates to an array
 of `UIColor` objects. Set this property to `nil` to reset it to the default
 value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `UIColor` array values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeHighlightColor;
#else
/**
 The shading color of areas that faces towards the light source(s). Only when
 `hillshadeMethod` is set to `MLNHillshadeMethodMultidirectional` can you
 specify multiple light sources.
 
 The default value of this property is an expression that evaluates to an array
 of `NSColor` objects. Set this property to `nil` to reset it to the default
 value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `NSColor` array values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeHighlightColor;
#endif

/**
 The transition affecting any changes to this layer’s `hillshadeHighlightColor` property.

 This property corresponds to the `hillshade-highlight-color-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition hillshadeHighlightColorTransition;

/**
 The altitude of the light source(s) used to generate the hillshading with 0 as
 sunset and 90 as noon. Only when `hillshadeMethod` is set to
 `MLNHillshadeMethodMultidirectional` can you specify multiple light sources.
 
 The default value of this property is an expression that evaluates to an array
 of numeric values. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric array values between 0 and 90 inclusive
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeIlluminationAltitude;

/**
 Direction of light source when map is rotated.
 
 The default value of this property is an expression that evaluates to
 `viewport`. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `MLNHillshadeIlluminationAnchor` values
 * Any of the following constant string values:
   * `map`: The hillshade illumination is relative to the north direction.
   * `viewport`: The hillshade illumination is relative to the top of the
 viewport.
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation functions to the
 `$zoomLevel` variable or applying interpolation or step functions to feature
 attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeIlluminationAnchor;

/**
 The direction of the light source(s) used to generate the hillshading with 0 as
 the top of the viewport if `hillshadeIlluminationAnchor` is set to
 `MLNHillshadeIlluminationAnchorViewport` and due north if
 `hillshadeIlluminationAnchor` is set to `MLNHillshadeIlluminationAnchorMap`.
 Only when `hillshadeMethod` is set to `MLNHillshadeMethodMultidirectional` can
 you specify multiple light sources.
 
 The default value of this property is an expression that evaluates to an array
 of numeric values. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric array values between 0 and 359 inclusive
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeIlluminationDirection;

/**
 The hillshade algorithm to use, one of `MLNHillshadeMethodStandard`,
 `MLNHillshadeMethodBasic`, `MLNHillshadeMethodCombined`,
 `MLNHillshadeMethodIgor`, or `MLNHillshadeMethodMultidirectional`.
 ![image](assets/hillshade_methods.png)
 
 The default value of this property is an expression that evaluates to
 `standard`. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `MLNHillshadeMethod` values
 * Any of the following constant string values:
   * `standard`: The legacy hillshade method.
   * `basic`: Basic hillshade. Uses a simple physics model where the reflected
 light intensity is proportional to the cosine of the angle between the incident
 light and the surface normal. Similar to GDAL's `gdaldem` default algorithm.
   * `combined`: Hillshade algorithm whose intensity scales with slope. Similar
 to GDAL's `gdaldem` with `-combined` option.
   * `igor`: Hillshade algorithm which tries to minimize effects on other map
 features beneath. Similar to GDAL's `gdaldem` with `-igor` option.
   * `multidirectional`: Hillshade with multiple illumination directions. Uses
 the basic hillshade model with multiple independent light sources.
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation functions to the
 `$zoomLevel` variable or applying interpolation or step functions to feature
 attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeMethod;

#if TARGET_OS_IPHONE
/**
 The shading color of areas that face away from the light source(s). Only when
 `hillshadeMethod` is set to `MLNHillshadeMethodMultidirectional` can you
 specify multiple light sources.
 
 The default value of this property is an expression that evaluates to an array
 of `UIColor` objects. Set this property to `nil` to reset it to the default
 value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `UIColor` array values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeShadowColor;
#else
/**
 The shading color of areas that face away from the light source(s). Only when
 `hillshadeMethod` is set to `MLNHillshadeMethodMultidirectional` can you
 specify multiple light sources.
 
 The default value of this property is an expression that evaluates to an array
 of `NSColor` objects. Set this property to `nil` to reset it to the default
 value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `NSColor` array values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *hillshadeShadowColor;
#endif

/**
 The transition affecting any changes to this layer’s `hillshadeShadowColor` property.

 This property corresponds to the `hillshade-shadow-color-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition hillshadeShadowColorTransition;

@end

/**
 Methods for wrapping an enumeration value for a style layer attribute in an
 ``MLNHillshadeStyleLayer`` object and unwrapping its raw value.
 */
@interface NSValue (MLNHillshadeStyleLayerAdditions)

// MARK: Working with Hillshade Style Layer Attribute Values

/**
 Creates a new value object containing the given `MLNHillshadeIlluminationAnchor` enumeration.

 @param hillshadeIlluminationAnchor The value for the new object.
 @return A new value object that contains the enumeration value.
 */
+ (instancetype)valueWithMLNHillshadeIlluminationAnchor:(MLNHillshadeIlluminationAnchor)hillshadeIlluminationAnchor;

/**
 The ``MLNHillshadeIlluminationAnchor`` enumeration representation of the value.
 */
@property (readonly) MLNHillshadeIlluminationAnchor MLNHillshadeIlluminationAnchorValue;

/**
 Creates a new value object containing the given `MLNHillshadeMethod` enumeration.

 @param hillshadeMethod The value for the new object.
 @return A new value object that contains the enumeration value.
 */
+ (instancetype)valueWithMLNHillshadeMethod:(MLNHillshadeMethod)hillshadeMethod;

/**
 The ``MLNHillshadeMethod`` enumeration representation of the value.
 */
@property (readonly) MLNHillshadeMethod MLNHillshadeMethodValue;

@end

NS_ASSUME_NONNULL_END
