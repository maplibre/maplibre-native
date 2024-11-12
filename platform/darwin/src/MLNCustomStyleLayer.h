#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>

#import "MLNFoundation.h"
#import "MLNGeometry.h"
#import "MLNStyleLayer.h"
#import "MLNStyleValue.h"

#if MLN_RENDER_BACKEND_METAL
#import <MetalKit/MetalKit.h>
#endif

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
@interface MLNCustomStyleLayer : MLNStyleLayer

@property (nonatomic, weak, readonly) MLNStyle *style;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#if TARGET_OS_IPHONE
@property (nonatomic, readonly) EAGLContext *context;
#else
@property (nonatomic, readonly) CGLContextObj context;
#endif
#pragma clang diagnostic pop

#if MLN_RENDER_BACKEND_METAL
@property (nonatomic, weak) id<MTLRenderCommandEncoder> renderEncoder;
#endif

- (instancetype)initWithIdentifier:(NSString *)identifier;

- (void)didMoveToMapView:(MLNMapView *)mapView;

- (void)willMoveFromMapView:(MLNMapView *)mapView;

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context;

- (void)setNeedsDisplay;

@end

NS_ASSUME_NONNULL_END
