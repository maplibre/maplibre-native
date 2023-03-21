#import "MLNUserLocationAnnotationView.h"
#import "MLNUserLocation.h"

NS_ASSUME_NONNULL_BEGIN

@class MLNMapView;

@interface MLNUserLocationAnnotationView (Private)

@property (nonatomic, weak, nullable) MLNUserLocation *userLocation;
@property (nonatomic, weak, nullable) MLNMapView *mapView;

@end

NS_ASSUME_NONNULL_END
