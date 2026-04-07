// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNFoundation.h"
#import "MLNForegroundStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

/**
 Client-side elevation coloring based on DEM data. The implementation supports
 Mapbox Terrain RGB, Mapzen Terrarium tiles and custom encodings.

 You can access an existing color-relief style layer using the
 ``MLNStyle/layerWithIdentifier:`` method if you know its identifier;
 otherwise, find it using the ``MLNStyle/layers`` property. You can also create a
 new color-relief style layer and add it to the style using a method such as
 ``MLNStyle/addLayer:``.

 ### Example

 ```swift
 ```
 */
MLN_EXPORT
@interface MLNColorReliefStyleLayer : MLNForegroundStyleLayer

/**
 Returns a color-relief style layer initialized with an identifier and source.

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
 Defines the color of each point based on its elevation. Should be an expression
 that uses `["elevation"]` as input.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `UIColor` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$elevation` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *colorReliefColor;
#else
/**
 Defines the color of each point based on its elevation. Should be an expression
 that uses `["elevation"]` as input.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `NSColor` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$elevation` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *colorReliefColor;
#endif

/**
 The opacity at which the color-relief will be drawn.
 
 The default value of this property is an expression that evaluates to the float
 `1`. Set this property to `nil` to reset it to the default value.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric values between 0 and 1 inclusive
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.
 */
@property (nonatomic, null_resettable) NSExpression *colorReliefOpacity;

/**
 The transition affecting any changes to this layer’s `colorReliefOpacity` property.

 This property corresponds to the `color-relief-opacity-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition colorReliefOpacityTransition;

@end

NS_ASSUME_NONNULL_END
