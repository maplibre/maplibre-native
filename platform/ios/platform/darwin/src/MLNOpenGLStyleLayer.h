#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>
#import <QuartzCore/QuartzCore.h>

#import "MLNFoundation.h"
#import "MLNStyleValue.h"
#import "MLNStyleLayer.h"
#import "MLNGeometry.h"

NS_ASSUME_NONNULL_BEGIN

@class MLNMapView;
@class MLNStyle;

typedef struct MLNStyleLayerDrawingContext {
    CGSize size;
    CLLocationCoordinate2D centerCoordinate;
    double zoomLevel;
    CLLocationDirection direction;
    CGFloat pitch;
    CGFloat fieldOfView;
    MLNMatrix4 projectionMatrix;
} MLNStyleLayerDrawingContext;

MLN_EXPORT
@interface MLNOpenGLStyleLayer : MLNStyleLayer

@property (nonatomic, weak, readonly) MLNStyle *style;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#if TARGET_OS_IPHONE
@property (nonatomic, readonly) EAGLContext *context;
#else
@property (nonatomic, readonly) CGLContextObj context;
#endif
#pragma clang diagnostic pop

- (instancetype)initWithIdentifier:(NSString *)identifier;

- (void)didMoveToMapView:(MLNMapView *)mapView;

- (void)willMoveFromMapView:(MLNMapView *)mapView;

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context;

- (void)setNeedsDisplay;

@end

NS_ASSUME_NONNULL_END
