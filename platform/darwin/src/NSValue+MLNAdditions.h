#import <Foundation/Foundation.h>

#import "MLNGeometry.h"
#import "MLNLight.h"
#import "MLNOfflinePack.h"
#import "MLNTypes.h"

NS_ASSUME_NONNULL_BEGIN

/**
 Methods for round-tripping values for Mapbox-defined types.
 */
@interface NSValue (MLNAdditions)

// MARK: Working with Geographic Coordinate Values

/**
 Creates a new value object containing the specified Core Location geographic
 coordinate structure.

 @param coordinate The value for the new object.
 @return A new value object that contains the geographic coordinate information.
 */
+ (instancetype)valueWithMLNCoordinate:(CLLocationCoordinate2D)coordinate;

/**
 The Core Location geographic coordinate structure representation of the value.
 */
@property (readonly) CLLocationCoordinate2D MLNCoordinateValue;

/**
 Creates a new value object containing the specified Mapbox map point structure.

 @param point The value for the new object.
 @return A new value object that contains the coordinate and zoom level information.
 */
+ (instancetype)valueWithMLNMapPoint:(MLNMapPoint)point;

/**
 The Mapbox map point structure representation of the value.
 */
@property (readonly) MLNMapPoint MLNMapPointValue;

/**
 Creates a new value object containing the specified Mapbox coordinate span
 structure.

 @param span The value for the new object.
 @return A new value object that contains the coordinate span information.
 */
+ (instancetype)valueWithMLNCoordinateSpan:(MLNCoordinateSpan)span;

/**
 The Mapbox coordinate span structure representation of the value.
 */
@property (readonly) MLNCoordinateSpan MLNCoordinateSpanValue;

/**
 Creates a new value object containing the specified Mapbox coordinate bounds
 structure.

 @param bounds The value for the new object.
 @return A new value object that contains the coordinate bounds information.
 */
+ (instancetype)valueWithMLNCoordinateBounds:(MLNCoordinateBounds)bounds;

/**
 The Mapbox coordinate bounds structure representation of the value.
 */
@property (readonly) MLNCoordinateBounds MLNCoordinateBoundsValue;

/**
 Creates a new value object containing the specified Mapbox coordinate
 quad structure.

 @param quad The value for the new object.
 @return A new value object that contains the coordinate quad information.
 */
+ (instancetype)valueWithMLNCoordinateQuad:(MLNCoordinateQuad)quad;

/**
 The Mapbox coordinate quad structure representation of the value.
 */
- (MLNCoordinateQuad)MLNCoordinateQuadValue;

// MARK: Working with Offline Map Values

/**
 Creates a new value object containing the given ``MLNOfflinePackProgress``
 structure.

 @param progress The value for the new object.
 @return A new value object that contains the offline pack progress information.
 */
+ (NSValue *)valueWithMLNOfflinePackProgress:(MLNOfflinePackProgress)progress;

/**
 The ``MLNOfflinePackProgress`` structure representation of the value.
 */
@property (readonly) MLNOfflinePackProgress MLNOfflinePackProgressValue;

// MARK: Working with Transition Values

/**
 Creates a new value object containing the given ``MLNTransition``
 structure.

 @param transition The value for the new object.
 @return A new value object that contains the transition information.
 */
+ (NSValue *)valueWithMLNTransition:(MLNTransition)transition;

/**
 The ``MLNTransition`` structure representation of the value.
 */
@property (readonly) MLNTransition MLNTransitionValue;

/**
 Creates a new value object containing the given ``MLNSphericalPosition``
 structure.

 @param lightPosition The value for the new object.
 @return A new value object that contains the light position information.
 */
+ (instancetype)valueWithMLNSphericalPosition:(MLNSphericalPosition)lightPosition;

/**
 The ``MLNSphericalPosition`` structure representation of the value.
 */
@property (readonly) MLNSphericalPosition MLNSphericalPositionValue;

/**
 Creates a new value object containing the given ``MLNLightAnchor``
 enum.

 @param lightAnchor The value for the new object.
 @return A new value object that contains the light anchor information.
 */
+ (NSValue *)valueWithMLNLightAnchor:(MLNLightAnchor)lightAnchor;

/**
 The ``MLNLightAnchor`` enum representation of the value.
 */
@property (readonly) MLNLightAnchor MLNLightAnchorValue;

@end

NS_ASSUME_NONNULL_END
