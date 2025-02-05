#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MLNLocationManagerDelegate;

/**
 The ``MLNLocationManager`` protocol defines a set of methods that a class must
 implement in order to serve as the location manager of an ``MLNMapView``. A location
 manager is responsible for notifying the map view about location-related events,
 such as a change in the user’s location. This protocol is similar to the
 Core Location framework’s `CLLocationManager` class, but your implementation
 does not need to be based on `CLLocationManager`.

 To receive location updates from an object that conforms to the ``MLNLocationManager``
 protocol, use the optional methods available in the ``MLNLocationManagerDelegate`` protocol.
 */
@protocol MLNLocationManager <NSObject>

@optional

// MARK: Configuring Location Update Precision

/**
 Specifies the minimum distance (measured in meters) a device must move horizontally
 before a location update is generated.

 The default value of this property is `kCLDistanceFilterNone` when ``MLNMapView`` uses its
 default location manager.

 @see `CLLocationManager.distanceFilter`
 */
- (CLLocationDistance)distanceFilter;

/**
 Sets the minimum update distance in meters.
 @param distanceFilter The distance filter in meters.
 */
- (void)setDistanceFilter:(CLLocationDistance)distanceFilter;

/**
 Specifies the accuracy of the location data.

 The default value is `kCLLocationAccuracyBest` when ``MLNMapView`` uses its
 default location manager.

 > Note: Determining a location with greater accuracy requires more time and more power.

 @see `CLLocationManager.desiredAccuracy`
 */
- (CLLocationAccuracy)desiredAccuracy;

/**
 Sets the desired location accuracy.

 @param desiredAccuracy The desired location accuracy.
 */
- (void)setDesiredAccuracy:(CLLocationAccuracy)desiredAccuracy;

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000
/**
 Specifies the level of location accuracy the Maps SDK has permission to use.

 > Note: If the value of this property is `CLAccuracyAuthorizationFullAccuracy`, you can set the
 ``MLNLocationManager/desiredAccuracy`` property to any value. If the value is
 `CLAccuracyAuthorizationReducedAccuracy`, setting ``MLNLocationManager/desiredAccuracy`` to a value
 other than` kCLLocationAccuracyReduced` has no effect on the location information.
 */
- (CLAccuracyAuthorization)accuracyAuthorization API_AVAILABLE(ios(14));
#endif

/**
 Specifies the type of user activity associated with the location updates.

 The location manager uses this property as a cue to determine when location updates
 may be automatically paused.

 The default value is `CLActivityTypeOther` when ``MLNMapView`` uses its
 default location manager.

 @see `CLLocationManager.activityType`
 */
- (CLActivityType)activityType;

/**
 Sets the type of user activity associated with the location updates.

 @param activityType The location's manager activity type.
 */
- (void)setActivityType:(CLActivityType)activityType;

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000
/**
 Requests the user's permission to temporarily use location update services
 with full accuracy.

 > Note: If the user turned off location accuracy you may use this method to
 request full accuracy for a session.
 */
- (void)requestTemporaryFullAccuracyAuthorizationWithPurposeKey:(NSString *)purposeKey
    API_AVAILABLE(ios(14));
#endif

@required

/**
 The delegate to receive location updates.

 Do not set the location manager’s delegate yourself. ``MLNMapView`` sets this property
 after the location manager becomes ``MLNMapView``’s location manager.
 */
@property (nonatomic, weak) id<MLNLocationManagerDelegate> delegate;

// MARK: Requesting Authorization for Location Services

/**
 Returns the current localization authorization status.

 @see `+[CLLocationManger authorizationStatus]`
 */
@property (nonatomic, readonly) CLAuthorizationStatus authorizationStatus;

/**
 Requests permission to use the location services whenever the app is running.
 */
- (void)requestAlwaysAuthorization;

/**
 Requests permission to use the location services while the app is in
 the foreground.
 */
- (void)requestWhenInUseAuthorization;

// MARK: Initiating Location Updates

/**
 Starts the generation of location updates that reports the user's current location.
 */
- (void)startUpdatingLocation;

/**
 Stops the generation of location updates.
 */
- (void)stopUpdatingLocation;

// MARK: Initiating Heading Updates

/**
 Specifies a physical device orientation.
 */
@property (nonatomic) CLDeviceOrientation headingOrientation;

/**
 Starts the generation of heading updates that reports the user's current hading.
 */
- (void)startUpdatingHeading;

#if TARGET_OS_IPHONE
/**
 Stops the generation of heading updates.
 */
- (void)stopUpdatingHeading;
#endif

/**
 Dissmisses immediately the heading calibration view from screen.
 */
- (void)dismissHeadingCalibrationDisplay;

@end

/**
 The ``MLNLocationManagerDelegate`` protocol defines a set of methods that respond
 to location updates from an ``MLNLocationManager`` object that is serving as the
 location manager of an ``MLNMapView``.
 */
@protocol MLNLocationManagerDelegate <NSObject>

// MARK: Responding to Location Updates

/**
 Notifies the delegate with the new location data.

 @param manager The location manager reporting the update.
 @param locations An array of `CLLocation` objects in chronological order,
 with the last object representing the most recent location. This array
 contains multiple `CLLocation` objects when ``MLNMapView`` uses  its
 default location manager.
 */
- (void)locationManager:(id<MLNLocationManager>)manager
     didUpdateLocations:(NSArray<CLLocation *> *)locations;

// MARK: Responding to Heading Updates

/**
 Notifies the delegate with the new heading data.

 @param manager The location manager reporting the update.
 @param newHeading The new heading update.
 */
- (void)locationManager:(id<MLNLocationManager>)manager didUpdateHeading:(CLHeading *)newHeading;

/**
 Asks the delegate if the calibration alert should be displayed.

 @param manager The location manager reporting the calibration.
 */
- (BOOL)locationManagerShouldDisplayHeadingCalibration:(id<MLNLocationManager>)manager;

// MARK: Responding to Location Updates Errors

/**
 Notifies the delegate that the location manager was unable to retrieve
 location updates.

 @param manager The location manager reporting the error.
 @param error An error object containing the error code that indicates
 why the location manager failed.
 */
- (void)locationManager:(id<MLNLocationManager>)manager didFailWithError:(nonnull NSError *)error;
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000
/**
 Notifies the delegate that the location authorization status has changed.

 @param manager The location manager reporting the change.
 */
- (void)locationManagerDidChangeAuthorization:(id<MLNLocationManager>)manager;
#endif

@optional

@end

NS_ASSUME_NONNULL_END
