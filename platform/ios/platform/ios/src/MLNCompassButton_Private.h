#import <UIKit/UIKit.h>

#import "MLNCompassButton.h"

@class MLNMapView;

NS_ASSUME_NONNULL_BEGIN

@interface MLNCompassButton (Private)

+ (instancetype)compassButtonWithMapView:(MLNMapView *)mapView;

@property (nonatomic, weak) MLNMapView *mapView;

- (void)updateCompass;

@end

NS_ASSUME_NONNULL_END
