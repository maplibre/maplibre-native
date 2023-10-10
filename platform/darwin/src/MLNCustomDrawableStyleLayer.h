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

MLN_EXPORT
@interface MLNCustomDrawableStyleLayer : MLNStyleLayer

@property (nonatomic, weak, readonly) MLNStyle *style;

- (instancetype)initWithIdentifier:(NSString *)identifier;

- (void)didMoveToMapView:(MLNMapView *)mapView;

- (void)willMoveFromMapView:(MLNMapView *)mapView;

- (void)setNeedsDisplay;

@end

NS_ASSUME_NONNULL_END
