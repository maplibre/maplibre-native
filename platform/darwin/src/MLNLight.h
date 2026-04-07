// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import <CoreLocation/CoreLocation.h>

#import "MLNFoundation.h"
#import "MLNStyleValue.h"

NS_ASSUME_NONNULL_BEGIN

/**
 Whether extruded geometries are lit relative to the map or viewport.
 */
typedef NS_ENUM(NSUInteger, MLNLightAnchor) {
    /**
     The position of the light source is aligned to the rotation of the map.
     */
    MLNLightAnchorMap,
    /**
     The position of the light source is aligned to the rotation of the
     viewport.
     */
    MLNLightAnchorViewport,
};

/**
 A structure containing information about the position of the light source
 relative to lit geometries.
 */
typedef struct __attribute__((objc_boxable)) MLNSphericalPosition {
    /** Distance from the center of the base of an object to its light. */
    CGFloat radial;
    /** Position of the light relative to 0° (0° when ``MLNLight/anchor`` is set to viewport corresponds
     to the top of the viewport, or 0° when ``MLNLight/anchor`` is set to map corresponds to due north,
     and degrees proceed clockwise). */
    CLLocationDirection azimuthal;
    /** Indicates the height of the light (from 0°, directly above, to 180°, directly below). */
    CLLocationDirection polar;
} MLNSphericalPosition;

/**
 Creates a new ``MLNSphericalPosition`` from the given radial, azimuthal, polar.

 @param radial The radial coordinate.
 @param azimuthal The azimuthal angle.
 @param polar The polar angle.

 @return Returns a ``MLNSphericalPosition`` struct containing the position attributes.
 */
NS_INLINE MLNSphericalPosition MLNSphericalPositionMake(CGFloat radial, CLLocationDirection azimuthal, CLLocationDirection polar) {
    MLNSphericalPosition position;
    position.radial = radial;
    position.azimuthal = azimuthal;
    position.polar = polar;

    return position;
}

/**
  An ``MLNLight`` object represents the light source for extruded geometries in
 ``MLNLight``.
 
 ### Example
 ```swift
 ```
 
 #### Related examples
 See the <a
 href="https://docs.mapbox.com/ios/maps/examples/light-example/">Adjust light of
 3D buildings</a> to learn how to create and modify the light source for 3D
 geometries.
 */
MLN_EXPORT
@interface MLNLight : NSObject

/**
 Whether extruded geometries are lit relative to the map or viewport.
 
 The default value of this property is an expression that evaluates to
 `viewport`.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `MLNAnchor` values
 * Any of the following constant string values:
   * `map`: The position of the light source is aligned to the rotation of the
 map.
   * `viewport`: The position of the light source is aligned to the rotation of
 the viewport.
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation functions to the
 `$zoomLevel` variable or applying interpolation or step functions to feature
 attributes.

 This property corresponds to the <a
 href="https://maplibre.org/maplibre-style-spec/light/#anchor"><code>anchor</code></a>
 light property in the MapLibre Style Spec.
 */
@property (nonatomic) NSExpression *anchor;

/**
 Position of the ``MLNLight`` source relative to lit (extruded) geometries, in a
 ``MLNLight`` struct [radial coordinate, azimuthal angle, polar angle] where
 radial indicates the distance from the center of the base of an object to its
 light, azimuthal indicates the position of the light relative to 0° (0° when
 ``MLNLight/anchor`` is set to ``MLNLight`` corresponds to the top of the
 viewport, or 0° when ``MLNLight/anchor`` is set to ``MLNLight`` corresponds to
 due north, and degrees proceed clockwise), and polar indicates the height of
 the light (from 0°, directly above, to 180°, directly below).
 
 The default value of this property is an expression that evaluates to an
 ``MLNSphericalPosition`` struct set to 1.15 radial, 210 azimuthal and 30 polar.
 
 You can set this property to an expression containing any of the following:
 
 * Constant ``MLNSphericalPosition`` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.

 This property corresponds to the <a
 href="https://maplibre.org/maplibre-style-spec/light/#position"><code>position</code></a>
 light property in the MapLibre Style Spec.

 #### Related examples
 See the <a
 href="https://docs.mapbox.com/ios/maps/examples/light-example/">Adjust light of
 3D buildings</a> example to learn how to create and modify the position of
 value of an ``MLNLight`` object for 3D geometries.
 */
@property (nonatomic) NSExpression *position;

/**
 The transition affecting any changes to this layer’s `position` property.

 This property corresponds to the `position-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition positionTransition;

#if TARGET_OS_IPHONE
/**
 Color tint for lighting extruded geometries.
 
 The default value of this property is an expression that evaluates to
 `UIColor.whiteColor`.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `UIColor` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.

 This property corresponds to the <a
 href="https://maplibre.org/maplibre-style-spec/light/#color"><code>color</code></a>
 light property in the MapLibre Style Spec.
 */
@property (nonatomic) NSExpression *color;
#else
/**
 Color tint for lighting extruded geometries.
 
 The default value of this property is an expression that evaluates to
 `NSColor.whiteColor`.
 
 You can set this property to an expression containing any of the following:
 
 * Constant `NSColor` values
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.

 This property corresponds to the <a
 href="https://maplibre.org/maplibre-style-spec/light/#color"><code>color</code></a>
 light property in the MapLibre Style Spec.
 */
@property (nonatomic) NSExpression *color;
#endif

/**
 The transition affecting any changes to this layer’s `color` property.

 This property corresponds to the `color-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition colorTransition;

/**
 Intensity of lighting (on a scale from 0 to 1). Higher numbers will present as
 more extreme contrast.
 
 The default value of this property is an expression that evaluates to the float
 `0.5`.
 
 You can set this property to an expression containing any of the following:
 
 * Constant numeric values between 0 and 1 inclusive
 * Predefined functions, including mathematical and string operators
 * Conditional expressions
 * Variable assignments and references to assigned variables
 * Interpolation and step functions applied to the `$zoomLevel` variable
 
 This property does not support applying interpolation or step functions to
 feature attributes.

 This property corresponds to the <a
 href="https://maplibre.org/maplibre-style-spec/light/#intensity"><code>intensity</code></a>
 light property in the MapLibre Style Spec.
 */
@property (nonatomic) NSExpression *intensity;

/**
 The transition affecting any changes to this layer’s `intensity` property.

 This property corresponds to the `intensity-transition` property in the style JSON file format.
*/
@property (nonatomic) MLNTransition intensityTransition;


@end

NS_ASSUME_NONNULL_END
