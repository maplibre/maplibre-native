#import "MLNUserLocation.h"

#import <CoreLocation/CoreLocation.h>

@class MLNMapView;

NS_ASSUME_NONNULL_BEGIN

@interface MLNUserLocation (Private)

@property (nonatomic, weak) MLNMapView *mapView;
@property (nonatomic, readwrite, nullable) CLLocation *location;
@property (nonatomic, readwrite, nullable) CLHeading *heading;

- (instancetype)initWithMapView:(MLNMapView *)mapView;

@end

NS_ASSUME_NONNULL_END
