#import "MLNGeometry.h"

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE
    #import <UIKit/UIKit.h>
#endif

#import <mbgl/util/geo.hpp>
#import <mbgl/util/geometry.hpp>

#import <array>
typedef double MLNLocationRadians;
typedef double MLNRadianDistance;
typedef double MLNRadianDirection;

/** Defines the coordinate by a `MLNRadianCoordinate2D`. */
typedef struct MLNRadianCoordinate2D {
    MLNLocationRadians latitude;
    MLNLocationRadians longitude;
} MLNRadianCoordinate2D;

/**
 Creates a new `MLNRadianCoordinate2D` from the given latitudinal and longitudinal.
 */
NS_INLINE MLNRadianCoordinate2D MLNRadianCoordinate2DMake(MLNLocationRadians latitude, MLNLocationRadians longitude) {
    MLNRadianCoordinate2D radianCoordinate;
    radianCoordinate.latitude = latitude;
    radianCoordinate.longitude = longitude;
    return radianCoordinate;
}

/// Returns the smallest rectangle that contains both the given rectangle and
/// the given point.
CGRect MLNExtendRect(CGRect rect, CGPoint point);

#if TARGET_OS_IPHONE
NS_INLINE NSString *MLNStringFromSize(CGSize size) {
    return NSStringFromCGSize(size);
}
#else
NS_INLINE NSString *MLNStringFromSize(NSSize size) {
    return NSStringFromSize(size);
}
#endif

NS_INLINE NSString *MLNStringFromCLLocationCoordinate2D(CLLocationCoordinate2D coordinate) {
    return [NSString stringWithFormat:@"(lat: %f, lon: %f)", coordinate.latitude, coordinate.longitude];
}

mbgl::LatLng MLNLatLngFromLocationCoordinate2D(CLLocationCoordinate2D coordinate);

NS_INLINE mbgl::Point<double> MLNPointFromLocationCoordinate2D(CLLocationCoordinate2D coordinate) {
    return mbgl::Point<double>(coordinate.longitude, coordinate.latitude);
}

NS_INLINE CLLocationCoordinate2D MLNLocationCoordinate2DFromPoint(mbgl::Point<double> point) {
    return CLLocationCoordinate2DMake(point.y, point.x);
}

NS_INLINE CLLocationCoordinate2D MLNLocationCoordinate2DFromLatLng(mbgl::LatLng latLng) {
    return CLLocationCoordinate2DMake(latLng.latitude(), latLng.longitude());
}

NS_INLINE MLNCoordinateBounds MLNCoordinateBoundsFromLatLngBounds(mbgl::LatLngBounds latLngBounds) {
    return MLNCoordinateBoundsMake(MLNLocationCoordinate2DFromLatLng(latLngBounds.southwest()),
                                   MLNLocationCoordinate2DFromLatLng(latLngBounds.northeast()));
}

NS_INLINE mbgl::LatLngBounds MLNLatLngBoundsFromCoordinateBounds(MLNCoordinateBounds coordinateBounds) {
    return mbgl::LatLngBounds::hull(MLNLatLngFromLocationCoordinate2D(coordinateBounds.sw),
                                    MLNLatLngFromLocationCoordinate2D(coordinateBounds.ne));
}

NS_INLINE std::array<mbgl::LatLng, 4> MLNLatLngArrayFromCoordinateQuad(MLNCoordinateQuad quad) {
    return { MLNLatLngFromLocationCoordinate2D(quad.topLeft),
    MLNLatLngFromLocationCoordinate2D(quad.topRight),
    MLNLatLngFromLocationCoordinate2D(quad.bottomRight),
    MLNLatLngFromLocationCoordinate2D(quad.bottomLeft) };
}

NS_INLINE MLNCoordinateQuad MLNCoordinateQuadFromLatLngArray(std::array<mbgl::LatLng, 4> quad) {
    return { MLNLocationCoordinate2DFromLatLng(quad[0]),
    MLNLocationCoordinate2DFromLatLng(quad[3]),
    MLNLocationCoordinate2DFromLatLng(quad[2]),
    MLNLocationCoordinate2DFromLatLng(quad[1]) };
}

/**
 YES if the coordinate is valid or NO if it is not.
 Considers extended coordinates.
 */
NS_INLINE BOOL MLNLocationCoordinate2DIsValid(CLLocationCoordinate2D coordinate) {
    return (coordinate.latitude  <= 90.0  &&
            coordinate.latitude  >= -90.0  &&
            coordinate.longitude <= 360.0 &&
            coordinate.longitude >= -360.0);
}

#if TARGET_OS_IPHONE
    #define MLNEdgeInsets UIEdgeInsets
    #define MLNEdgeInsetsMake UIEdgeInsetsMake
#else
    #define MLNEdgeInsets NSEdgeInsets
    #define MLNEdgeInsetsMake NSEdgeInsetsMake
#endif

NS_INLINE mbgl::EdgeInsets MLNEdgeInsetsFromNSEdgeInsets(MLNEdgeInsets insets) {
    return { insets.top, insets.left, insets.bottom, insets.right };
}

NS_INLINE MLNEdgeInsets NSEdgeInsetsFromMLNEdgeInsets(const mbgl::EdgeInsets& insets) {
    return MLNEdgeInsetsMake(insets.top(), insets.left(), insets.bottom(), insets.right());
}

/// Returns the combination of two edge insets.
NS_INLINE MLNEdgeInsets MLNEdgeInsetsInsetEdgeInset(MLNEdgeInsets base, MLNEdgeInsets inset) {
    return MLNEdgeInsetsMake(base.top + inset.top,
                             base.left + inset.left,
                             base.bottom + inset.bottom,
                             base.right + inset.right);
}

/** Returns MLNRadianCoordinate2D, converted from CLLocationCoordinate2D. */
NS_INLINE MLNRadianCoordinate2D MLNRadianCoordinateFromLocationCoordinate(CLLocationCoordinate2D locationCoordinate) {
    return MLNRadianCoordinate2DMake(MLNRadiansFromDegrees(locationCoordinate.latitude),
                                     MLNRadiansFromDegrees(locationCoordinate.longitude));
}

/**
 Returns the distance in radians given two coordinates.
 */
MLNRadianDistance MLNDistanceBetweenRadianCoordinates(MLNRadianCoordinate2D from, MLNRadianCoordinate2D to);

/**
 Returns direction in radians given two coordinates.
 */
MLNRadianDirection MLNRadianCoordinatesDirection(MLNRadianCoordinate2D from, MLNRadianCoordinate2D to);

/**
 Returns a coordinate at a given distance and direction away from coordinate.
 */
MLNRadianCoordinate2D MLNRadianCoordinateAtDistanceFacingDirection(MLNRadianCoordinate2D coordinate,
                                                                   MLNRadianDistance distance,
                                                                   MLNRadianDirection direction);

/**
 Returns the direction from one coordinate to another.
 */
CLLocationDirection MLNDirectionBetweenCoordinates(CLLocationCoordinate2D firstCoordinate, CLLocationCoordinate2D secondCoordinate);

/**
 Returns a point with coordinates rounded to the nearest logical pixel.
 */
CGPoint MLNPointRounded(CGPoint point);

MLNMatrix4 MLNMatrix4Make(std::array<double, 16> mat);
