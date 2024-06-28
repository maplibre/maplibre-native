#import "MLNFoundation.h"
#import "MLNMapView.h"

NS_ASSUME_NONNULL_BEGIN

/**
 The aim of this class is to provide the functionality of changing the camera state and
 converting between map view screen coordinates and geographical coordinates without
 changing the actual map view camera state.
*/
MLN_EXPORT
@interface MLNMapProjection : NSObject

/**
 Initializes and returns the new projection object with the current
 camera state from the provided map view.

 @param mapView The map view the camera state to use for the initialization.
 @return An initialized map projection.
 */
- (instancetype)initWithMapView:(MLNMapView *)mapView;

/**
 A camera representing the current projection state
 */
@property (readonly, copy) MLNMapCamera *camera;

/**
 Change the projection state with camera and padding values.

 @param camera The new camera to be used in the projection calculation.
 @param insets The insets applied on top of the camera be used in the projection calculation.

 @note `MLNMapView` instance frame must not be changed since this projection is initialized,
        otherwise the calculation may be wrong.
 */
- (void)setCamera:(MLNMapCamera *_Nonnull)camera withEdgeInsets:(UIEdgeInsets)insets;

/**
 Change the projection state to make the provided bounds visible with the specified inset.

 @param bounds The bounds that the viewport should fit.
 @param insets The insets applied on top of the viewport to be used in the projection calculation.

 @note `MLNMapView` instance frame must not be changed since this projection is initialized,
     otherwise the calculation may be wrong.
 */
- (void)setVisibleCoordinateBounds:(MLNCoordinateBounds)bounds edgePadding:(UIEdgeInsets)insets;

/**
 Converts a point in the coordinate system of the map view the projection
 was initialized with to the geographical coordinate.

 @param point The point to convert.
 @return The geographic coordinate at the given point.
 */
- (CLLocationCoordinate2D)convertPoint:(CGPoint)point;

/**
 Converts a geographic coordinate to a point in the map view's the projection
 was initialized with coordinate system.

 @param coordinate The geographic coordinate to convert.
 @return The point corresponding to the given geographic coordinate.
 */
- (CGPoint)convertCoordinate:(CLLocationCoordinate2D)coordinate;

/**
 The distance in meters spanned by a single point for the current camera.
 */
@property (readonly) CLLocationDistance metersPerPoint;

@end

NS_ASSUME_NONNULL_END
