#import "NSValue+MLNAdditions.h"

@implementation NSValue (MLNAdditions)

// MARK: Geometry

+ (instancetype)valueWithMLNCoordinate:(CLLocationCoordinate2D)coordinate {
    return [self valueWithBytes:&coordinate objCType:@encode(CLLocationCoordinate2D)];
}

- (CLLocationCoordinate2D)MLNCoordinateValue {
    CLLocationCoordinate2D coordinate;
    [self getValue:&coordinate];
    return coordinate;
}

+ (instancetype)valueWithMLNMapPoint:(MLNMapPoint)point {
    return [self valueWithBytes:&point objCType:@encode(MLNMapPoint)];
}

-(MLNMapPoint) MLNMapPointValue {
    MLNMapPoint point;
    [self getValue:&point];
    return point;
}

+ (instancetype)valueWithMLNCoordinateSpan:(MLNCoordinateSpan)span {
    return [self valueWithBytes:&span objCType:@encode(MLNCoordinateSpan)];
}

- (MLNCoordinateSpan)MLNCoordinateSpanValue {
    MLNCoordinateSpan span;
    [self getValue:&span];
    return span;
}

+ (instancetype)valueWithMLNCoordinateBounds:(MLNCoordinateBounds)bounds {
    return [self valueWithBytes:&bounds objCType:@encode(MLNCoordinateBounds)];
}

- (MLNCoordinateBounds)MLNCoordinateBoundsValue {
    MLNCoordinateBounds bounds;
    [self getValue:&bounds];
    return bounds;
}

+ (instancetype)valueWithMLNCoordinateQuad:(MLNCoordinateQuad)quad {
    return [self valueWithBytes:&quad objCType:@encode(MLNCoordinateQuad)];
}

- (MLNCoordinateQuad)MLNCoordinateQuadValue {
    MLNCoordinateQuad quad;
    [self getValue:&quad];
    return quad;
}

// MARK: Offline maps

+ (NSValue *)valueWithMLNOfflinePackProgress:(MLNOfflinePackProgress)progress {
    return [NSValue value:&progress withObjCType:@encode(MLNOfflinePackProgress)];
}

- (MLNOfflinePackProgress)MLNOfflinePackProgressValue {
    MLNOfflinePackProgress progress;
    [self getValue:&progress];
    return progress;
}

// MARK: Working with Transition Values

+ (NSValue *)valueWithMLNTransition:(MLNTransition)transition {
    return [NSValue value:&transition withObjCType:@encode(MLNTransition)];
}

- (MLNTransition)MLNTransitionValue {
    MLNTransition transition;
    [self getValue:&transition];
    return transition;
}

+ (NSValue *)valueWithMLNSphericalPosition:(MLNSphericalPosition)lightPosition
{
    return [NSValue value:&lightPosition withObjCType:@encode(MLNSphericalPosition)];
}

- (MLNSphericalPosition)MLNSphericalPositionValue
{
    MLNSphericalPosition lightPosition;
    [self getValue:&lightPosition];
    return lightPosition;
}

+ (NSValue *)valueWithMLNLightAnchor:(MLNLightAnchor)lightAnchor {
    return [NSValue value:&lightAnchor withObjCType:@encode(MLNLightAnchor)];
}

- (MLNLightAnchor)MLNLightAnchorValue
{
    MLNLightAnchor achorType;
    [self getValue:&achorType];
    return achorType;
}

@end
