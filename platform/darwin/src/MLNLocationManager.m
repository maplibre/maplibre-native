#import "MLNLocationManager_Private.h"

@interface MLNCLLocationManager()<CLLocationManagerDelegate>

@property (nonatomic) CLLocationManager *locationManager;

@end

@implementation MLNCLLocationManager

- (instancetype)init
{
    if (self = [super init]) {
        _locationManager = [[CLLocationManager alloc] init];
        _locationManager.delegate = self;
    }
    return self;
}

@synthesize delegate;

- (void)setHeadingOrientation:(CLDeviceOrientation)headingOrientation
{
    self.locationManager.headingOrientation = headingOrientation;
}

- (CLDeviceOrientation)headingOrientation
{
    return self.locationManager.headingOrientation;
}

- (void)setDesiredAccuracy:(CLLocationAccuracy)desiredAccuracy {
    self.locationManager.desiredAccuracy = desiredAccuracy;
}

- (CLLocationAccuracy)desiredAccuracy {
    return self.locationManager.desiredAccuracy;
}

- (CLAuthorizationStatus)authorizationStatus {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000
    if (@available(iOS 14.0, *)) {
        return self.locationManager.authorizationStatus;
    } else {
        return kCLAuthorizationStatusNotDetermined;
    }
#else
    return [CLLocationManager authorizationStatus];
#endif
}

- (void)setActivityType:(CLActivityType)activityType {
    self.locationManager.activityType = activityType;
}

- (CLActivityType)activityType {
    return self.locationManager.activityType;
}

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000
- (CLAccuracyAuthorization)accuracyAuthorization {
    if (@available(iOS 14.0, *)) {
        return self.locationManager.accuracyAuthorization;
    } else {
        return CLAccuracyAuthorizationFullAccuracy;
    }
}

- (void)requestTemporaryFullAccuracyAuthorizationWithPurposeKey:(NSString *)purposeKey {
    if (@available(iOS 14.0, *)) {
        [self.locationManager requestTemporaryFullAccuracyAuthorizationWithPurposeKey:purposeKey];
    } 
}
#endif


- (void)dismissHeadingCalibrationDisplay {
    [self.locationManager dismissHeadingCalibrationDisplay];
}

- (void)requestAlwaysAuthorization {
    [self.locationManager requestAlwaysAuthorization];
}

- (void)requestWhenInUseAuthorization {
    [self.locationManager requestWhenInUseAuthorization];
}

- (void)startUpdatingHeading {
    [self.locationManager startUpdatingHeading];
}

- (void)startUpdatingLocation {
    [self.locationManager startUpdatingLocation];
}

- (void)stopUpdatingHeading {
    [self.locationManager stopUpdatingHeading];
}

- (void)stopUpdatingLocation {
    [self.locationManager stopUpdatingLocation];
}

- (void)dealloc
{
    [self.locationManager stopUpdatingLocation];
    [self.locationManager stopUpdatingHeading];
    self.locationManager.delegate = nil;
    self.delegate = nil;
}

// MARK: - CLLocationManagerDelegate

- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray<CLLocation *> *)locations {
    if ([self.delegate respondsToSelector:@selector(locationManager:didUpdateLocations:)]) {
        [self.delegate locationManager:self didUpdateLocations:locations];
    }
}

- (void)locationManager:(CLLocationManager *)manager didUpdateHeading:(CLHeading *)newHeading {
    if ([self.delegate respondsToSelector:@selector(locationManager:didUpdateHeading:)]) {
        [self.delegate locationManager:self didUpdateHeading:newHeading];
    }
}

- (BOOL)locationManagerShouldDisplayHeadingCalibration:(CLLocationManager *)manager {
    if ([self.delegate respondsToSelector:@selector(locationManagerShouldDisplayHeadingCalibration:)]) {
        return [self.delegate locationManagerShouldDisplayHeadingCalibration:self];
    }
    
    return NO;
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error {
    if ([self.delegate respondsToSelector:@selector(locationManager:didFailWithError:)]) {
        [self.delegate locationManager:self didFailWithError:error];
    }
}

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000
- (void)locationManagerDidChangeAuthorization:(CLLocationManager *)manager {
    [self.delegate locationManagerDidChangeAuthorization:self];
}
#endif
@end
