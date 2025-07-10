#import <UIKit/UIKit.h>

#import "MLNBackendResource.h"
#import "MLNCompassButton.h"
#import "MLNFoundation.h"
#import "MLNGeometry.h"
#import "MLNMapCamera.h"
#import "MLNMapOptions.h"
#import "MLNStyle.h"
#import "MLNTypes.h"

NS_ASSUME_NONNULL_BEGIN

@class MLNAnnotationView;
@class MLNAnnotationImage;
@class MLNUserLocation;
@class MLNMapProjection;
@class MLNPolyline;
@class MLNPolygon;
@class MLNScaleBar;
@class MLNShape;
@class MLNPluginLayer;

@protocol MLNMapViewDelegate;
@protocol MLNAnnotation;
@protocol MLNOverlay;
@protocol MLNCalloutView;
@protocol MLNFeature;
@protocol MLNLocationManager;

/** Options for ``MLNMapView/decelerationRate``. */
typedef CGFloat MLNMapViewDecelerationRate NS_TYPED_EXTENSIBLE_ENUM;

/** The default deceleration rate for a map view. */
FOUNDATION_EXTERN MLN_EXPORT const MLNMapViewDecelerationRate MLNMapViewDecelerationRateNormal;

/** A fast deceleration rate for a map view. */
FOUNDATION_EXTERN MLN_EXPORT const MLNMapViewDecelerationRate MLNMapViewDecelerationRateFast;

/** Disables deceleration in a map view. */
FOUNDATION_EXTERN MLN_EXPORT const MLNMapViewDecelerationRate MLNMapViewDecelerationRateImmediate;

/**
 The vertical alignment of an annotation within a map view. Used with
 ``MLNMapView/userLocationVerticalAlignment``.
 */
typedef NS_ENUM(NSUInteger, MLNAnnotationVerticalAlignment) {
  /** Aligns the annotation vertically in the center of the map view. */
  MLNAnnotationVerticalAlignmentCenter = 0,
  /** Aligns the annotation vertically at the top of the map view. */
  MLNAnnotationVerticalAlignmentTop,
  /** Aligns the annotation vertically at the bottom of the map view. */
  MLNAnnotationVerticalAlignmentBottom,
};

/**
 The position of scale bar, compass, logo and attribution in a map view. Used with
 ``MLNMapView/scaleBarPosition``,
 ``MLNMapView/compassViewPosition``,
 ``MLNMapView/logoViewPosition``,
 ``MLNMapView/attributionButtonPosition``.
 */
typedef NS_ENUM(NSUInteger, MLNOrnamentPosition) {
  /** Place the ornament in the top left of the map view. */
  MLNOrnamentPositionTopLeft = 0,
  /** Place the ornament in the top right of the map view. */
  MLNOrnamentPositionTopRight,
  /** Place the ornament in the bottom left of the map view. */
  MLNOrnamentPositionBottomLeft,
  /** Place the ornament in the bottom right of the map view. */
  MLNOrnamentPositionBottomRight,
};

/**
 The mode used to track the user location on the map. Used with
 ``MLNMapView/userTrackingMode``.

 #### Related examples
 - TODO: Switch between user tracking modes</a> example to learn how to toggle modes and
 how each mode behaves.
 */
typedef NS_ENUM(NSUInteger, MLNUserTrackingMode) {
  /** The map does not follow the user location. */
  MLNUserTrackingModeNone = 0,
  /** The map follows the user location. This tracking mode falls back
      to ``MLNUserTrackingMode/MLNUserTrackingModeNone`` if the user pans the map view. */
  MLNUserTrackingModeFollow,
  /**
      The map follows the user location and rotates when the heading changes.
      The default user location annotation displays a fan-shaped indicator with
      the current heading. The heading indicator represents the direction the
      device is facing, which is sized according to the reported accuracy.

      This tracking mode is disabled if the user pans the map view, but
      remains enabled if the user zooms in. If the user rotates the map
      view, this tracking mode will fall back to ``MLNUserTrackingModeFollow``.
   */
  MLNUserTrackingModeFollowWithHeading,
  /**
      The map follows the user location and rotates when the course changes.
      Course represents the direction in which the device is traveling.
      The default user location annotation shows a puck-shaped indicator
      that rotates as the course changes.

      This tracking mode is disabled if the user pans the map view, but
      remains enabled if the user zooms in. If the user rotates the map view,
      this tracking mode will fall back to ``MLNUserTrackingModeFollow``.
   */
  MLNUserTrackingModeFollowWithCourse,
};

typedef NS_ENUM(NSUInteger, MLNPanScrollingMode) {
  /** The map allows the user to only scroll horizontally. */
  MLNPanScrollingModeHorizontal = 0,
  /** The map allows the user to only scroll vertically. */
  MLNPanScrollingModeVertical,
  /** The map allows the user to scroll both horizontally and vertically. */
  MLNPanScrollingModeDefault
};

/** Options for ``MLNMapView/preferredFramesPerSecond``. */
typedef NSInteger MLNMapViewPreferredFramesPerSecond NS_TYPED_EXTENSIBLE_ENUM;

/**
 The default frame rate. This can be either 30 FPS or 60 FPS, depending on
 device capabilities.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNMapViewPreferredFramesPerSecond
    MLNMapViewPreferredFramesPerSecondDefault;

/** A conservative frame rate; typically 30 FPS. */
FOUNDATION_EXTERN MLN_EXPORT const MLNMapViewPreferredFramesPerSecond
    MLNMapViewPreferredFramesPerSecondLowPower;

/** The maximum supported frame rate; typically 60 FPS. */
FOUNDATION_EXTERN MLN_EXPORT const MLNMapViewPreferredFramesPerSecond
    MLNMapViewPreferredFramesPerSecondMaximum;

FOUNDATION_EXTERN MLN_EXPORT
    MLNExceptionName const MLNMissingLocationServicesUsageDescriptionException;
FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNUserLocationAnnotationTypeException;

/**
 An interactive, customizable map view with an interface similar to the one
 provided by Apple’s MapKit.

 Using ``MLNMapView``, you can embed the map inside a view, allow users to
 manipulate it with standard gestures, animate the map between different
 viewpoints, and present information in the form of annotations and overlays.

 The map view loads scalable vector tiles that conform to the
 <a href="https://github.com/mapbox/vector-tile-spec">Mapbox Vector Tile Specification</a>.
 It styles them with a style that conforms to the
 <a href="https://maplibre.org/maplibre-style-spec/">MapLibre Style Spec</a>.
 Such styles can be designed with
 <a href="https://maplibre.org/maputnik/">Maputnik</a>.


 Because ``MLNMapView`` loads asynchronously, several delegate methods are available
 for receiving map-related updates. These methods can be used to ensure that certain operations
 have completed before taking any additional actions. Information on these methods is located
 in the ``MLNMapViewDelegate`` protocol documentation.

 Adding your own gesture recognizer to ``MLNMapView`` will block the corresponding
 gesture recognizer built into ``MLNMapView``. To avoid conflicts, define which
 gesture takes precedence. For example, you can create your own
 `UITapGestureRecognizer` that will be invoked only if the default ``MLNMapView``
 tap gesture fails:

 ```swift
 let mapTapGestureRecognizer = UITapGestureRecognizer(target: self, action:
 #selector(myCustomFunction)) for recognizer in mapView.gestureRecognizers! where recognizer is
 UITapGestureRecognizer { mapTapGestureRecognizer.require(toFail: recognizer)
 }
 mapView.addGestureRecognizer(mapTapGestureRecognizer)
 ```

 > Note: You are responsible for getting permission to use the map data and for
 ensuring that your use adheres to the relevant terms of use.
 */
MLN_EXPORT
@interface MLNMapView : UIView <MLNStylable>

// MARK: Creating Instances

/**
 Initializes and returns a newly allocated map view with the specified frame
 and the default style.

 @param frame The frame for the view, measured in points.
 @return An initialized map view.
 */
- (instancetype)initWithFrame:(CGRect)frame;

/**
 Initializes and returns a newly allocated map view with the specified frame
 and style URL.

 @param frame The frame for the view, measured in points.
 @param styleURL URL of the map style to display. The URL may be a full HTTP
    or HTTPS URL, a canonical URL or a path to a local file relative
    to the application’s resource path. Specify `nil` for the default style.
 @return An initialized map view.

 #### Related examples

 - TODO: initialize an ``MLNMapView`` with a custom style
 - TODO: how to initialize an ``MLNMapView`` with a third-party tile source
 */
- (instancetype)initWithFrame:(CGRect)frame styleURL:(nullable NSURL *)styleURL;

/**
 * Initializes and returns a newly allocated map view with the specified frame
 * and style JSON.
 *
 * @param frame The frame for the view, measured in points.
 * @param styleJSON JSON string of the map style to display. The JSON must conform to the
 *        <a href="https://maplibre.org/maplibre-style-spec/">MapLibre Style Specification</a>.
 *        Specify `nil` for the default style.
 * @return An initialized map view.
 */
- (instancetype)initWithFrame:(CGRect)frame styleJSON:(NSString *)styleJSON;

/**
 Initializes and returns a newly allocated map view with the specified frame
 and the default style.

 @param frame The frame for the view, measured in points.
 @param options The map instance options
 @return An initialized map view.
 */
- (instancetype)initWithFrame:(CGRect)frame options:(MLNMapOptions *)options;

// MARK: Accessing the Delegate

/**
 The receiver’s delegate.

 A map view sends messages to its delegate to notify it of changes to its
 contents or the viewpoint. The delegate also provides information about
 annotations displayed on the map, such as the styles to apply to individual
 annotations.
 */
@property (nonatomic, weak, nullable) IBOutlet id<MLNMapViewDelegate> delegate;

// MARK: Configuring the Map’s Appearance

/**
 The style currently displayed in the receiver.

 Unlike the `styleURL` property, this property is set to an object that allows
 you to manipulate every aspect of the style locally.

 If the style is loading, this property is set to `nil` until the style finishes
 loading. If the style has failed to load, this property is set to `nil`.
 Because the style loads asynchronously, you should manipulate it in the
 ``MLNMapViewDelegate/mapView:didFinishLoadingStyle:`` or
 ``MLNMapViewDelegate/mapViewDidFinishLoadingMap:`` method. It is not possible
 to manipulate the style before it has finished loading.
 */
@property (nonatomic, readonly, nullable) MLNStyle *style;

/**
 URL of the style currently displayed in the receiver.

 The URL may be a full HTTP or HTTPS URL, canonical URL, or
 a path to a local file relative to the application’s resource path.

 If you set this property to `nil`, the receiver will use the default style
 and this property will automatically be set to that style’s URL.

 If you want to modify the current style without replacing it outright, or if
 you want to introspect individual style attributes, use the `style` property.

 #### Related examples
 - TODO: change the style of a map at runtime.
 */
@property (nonatomic, null_resettable) NSURL *styleURL;

/**
 * The style JSON representation of the map.
 *
 * Setting this property results in an asynchronous style change. If you wish to know when the style
 * change is complete, observe the ``MLNMapViewDelegate/mapView:didFinishLoadingStyle:`` method
 * on ``MLNMapViewDelegate``.
 *
 * The JSON must conform to the
 * <a href="https://maplibre.org/maplibre-style-spec/">MapLibre Style Specification</a>.
 *
 * @throws NSInvalidArgumentException if styleJSON is nil or invalid JSON
 */
@property (nonatomic, copy) NSString *styleJSON;

/**
 Reloads the style.

 You do not normally need to call this method. The map view automatically
 responds to changes in network connectivity by reloading the style.

 This method does not bust the cache. Even if the style has recently changed on
 the server, calling this method does not necessarily ensure that the map view
 reflects those changes.
 */
- (IBAction)reloadStyle:(nullable id)sender;

/**
 A boolean value that indicates if whether the map view should automatically
 adjust its content insets.

 When this property is set to `YES` the map automatically updates its
 `contentInset` property to account for any area not covered by navigation bars,
 tab bars, toolbars, and other ancestors that obscure the map view.

 */
@property (assign) BOOL automaticallyAdjustsContentInset;

/**
 A Boolean value indicating whether the map may display scale information.

 The scale bar may not be shown at all zoom levels. The scale bar becomes visible
 when the maximum distance visible on the map view is less than 400 miles (800
 kilometers). The zoom level where this occurs depends on the latitude at the map
 view’s center coordinate, as well as the device screen width. At latitudes
 farther from the equator, the scale bar becomes visible at lower zoom levels.

 The unit of measurement is determined by the user's device locale.

 The view controlled by this property is available at `scaleBar`. The default value
 of this property is `NO`.
 */
@property (nonatomic, assign) BOOL showsScale;

/**
 A control indicating the scale of the map. The scale bar is positioned in the
 upper-left corner. Enable the scale bar via `showsScale`.
 */
@property (nonatomic, readonly) MLNScaleBar *scaleBar;

/**
 Sets whether the scale uses styles that make it easier to read on a dark styled map
 */
@property (nonatomic, assign) BOOL scaleBarShouldShowDarkStyles;

/**
 Sets whether the scale uses metric
 */
@property (nonatomic, assign) BOOL scaleBarUsesMetricSystem;

/**
 The position of the scale bar. The default value is
 ``MLNOrnamentPosition/MLNOrnamentPositionTopLeft``.
 */
@property (nonatomic, assign) MLNOrnamentPosition scaleBarPosition;

/**
 A `CGPoint` indicating the position offset of the scale bar.
 */
@property (nonatomic, assign) CGPoint scaleBarMargins;

/**
 A control indicating the map’s direction and allowing the user to manipulate
 the direction, positioned in the upper-right corner.
 */
@property (nonatomic, readonly) MLNCompassButton *compassView;

/**
 The position of the compass view. The default value is
 ``MLNOrnamentPosition/MLNOrnamentPositionTopRight``.
 */
@property (nonatomic, assign) MLNOrnamentPosition compassViewPosition;

/**
 A `CGPoint` indicating the position offset of the compass.
 */
@property (nonatomic, assign) CGPoint compassViewMargins;

/**
 A logo, the MapLibre logo by default, positioned in the lower-left corner.
 You are not required to display this, but some vector-sources may require attribution.
 */
@property (nonatomic, readonly) UIImageView *logoView;

/**
 The position of the logo view. The default value is
 ``MLNOrnamentPosition/MLNOrnamentPositionBottomLeft``.
 */
@property (nonatomic, assign) MLNOrnamentPosition logoViewPosition;

/**
 A `CGPoint` indicating the position offset of the logo.
 */
@property (nonatomic, assign) CGPoint logoViewMargins;

/**
 A view showing legally required copyright notices,
 positioned at the bottom-right of the map view.

 If you choose to reimplement this view, assign the `-showAttribution:` method
 as the action for your view to present the default notices and settings.

 > Note: Attribution is often required for many vector sources,
    OpenStreetMap data, or other data such as satellite or terrain
    data. If that applies to this map view, do not hide this view or remove
    any notices from it.

 */
@property (nonatomic, readonly) UIButton *attributionButton;

/**
 The position of the attribution button. The default value is
 ``MLNOrnamentPosition/MLNOrnamentPositionBottomRight``.
 */
@property (nonatomic, assign) MLNOrnamentPosition attributionButtonPosition;

/**
 A `CGPoint` indicating the position offset of the attribution.
 */
@property (nonatomic, assign) CGPoint attributionButtonMargins;

/**
 Show the attribution action sheet.

 This action is performed when the user taps on the attribution button provided
 by default via the `attributionButton` property. If you implement a custom
 attribution button, you should add this action to the button.
 */
- (IBAction)showAttribution:(id)sender;

/**
 The preferred frame rate at which the map view is rendered.

 The default value for this property is
 ``MLNMapViewPreferredFramesPerSecondDefault``, which will adaptively set the
 preferred frame rate based on the capability of the user’s device to maintain
 a smooth experience.

 In addition to the provided ``MLNMapViewPreferredFramesPerSecond`` options, this
 property can be set to arbitrary integer values.

 @see `CADisplayLink.preferredFramesPerSecond`
 */
@property (nonatomic, assign) MLNMapViewPreferredFramesPerSecond preferredFramesPerSecond;

/**
 A Boolean value indicating whether the map should prefetch tiles.

 When this property is set to `YES`, the map view prefetches tiles designed for
 a low zoom level and displays them until receiving more detailed tiles for the
 current zoom level. The prefetched tiles typically contain simplified versions
 of each shape, improving the map view’s perceived performance.

 The default value of this property is `YES`.
 */
@property (nonatomic, assign) BOOL prefetchesTiles;

/**
 A Boolean value indicating whether the map may cache tiles for different zoom levels or not.

 When this property is set to `YES`,  the map view consumes more memory and
 provide a smoother user experience when zoom in/out.

 The default value of this property is `YES`.
 */

@property (nonatomic, assign) BOOL tileCacheEnabled;

// MARK: Tile LOD controls

/**
 Camera based tile level of detail controls

 Minimum radius around the view point in unit of tiles in which the fine
 grained zoom level tiles are always used when performing LOD
 radius must be greater than 1 (At least 1 fine detailed tile is present)
 A smaller radius value may improve performance at the cost of quality (tiles away from
 camera use lower Zoom levels)
 */
@property (nonatomic, assign) double tileLodMinRadius;

/**
 Camera based tile level of detail controls

 Factor for the distance to the camera view point
 A value larger than 1 increases the distance to the camera view point reducing LOD
 Larger values may improve performance at the cost of quality (tiles away from camera
 use lower Zoom levels)
 */
@property (nonatomic, assign) double tileLodScale;

/**
 Camera based tile level of detail controls

 Pitch angle in radians above which LOD calculation is performed
 A smaller radius value may improve performance at the cost of quality
 */
@property (nonatomic, assign) double tileLodPitchThreshold;

/**
 Camera based tile level of detail controls

 Shift applied to the Zoom level during LOD calculation
 A negative value shifts the Zoom level to a coarser level reducing quality but improving
 performance A positive value shifts the Zoom level to a finer level increasing details but
 negatively affecting performance A value of zero (default) does not apply any shift to the Zoom
 level It is not recommended to change the default value unless performance is critical and the loss
 of quality is acceptable. A value of -1 reduces the number of displayed tiles by a factor of 4 on
 average It is recommended to first configure the pixelRatio before adjusting TileLodZoomShift.
 */
@property (nonatomic, assign) double tileLodZoomShift;

// MARK: Displaying the User’s Location

/**
 The object that this map view uses to start and stop the delivery of
 location-related updates.

 To receive the current user location, implement the
 ``MLNMapViewDelegate/mapView:didUpdateUserLocation:`` and
 ``MLNMapViewDelegate/mapView:didFailToLocateUserWithError:`` methods.

 If setting this property to `nil` or if no custom manager is provided this
 property is set to the default location manager.

 ``MLNMapView`` uses a default location manager. If you want to substitute your
 own location manager, you should do so by setting this property before setting
 `showsUserLocation` to `YES`. To restore the default location manager,
 set this property to `nil`.
 */
@property (nonatomic, null_resettable) id<MLNLocationManager> locationManager;

/**
 A Boolean value indicating whether the map may display the user location.

 Setting this property to `YES` causes the map view to use the Core Location
 framework to find the current location. As long as this property is `YES`, the
 map view continues to track the user’s location and update it periodically.

 This property does not indicate whether the user’s position is actually visible
 on the map, only whether the map view is allowed to display it. To determine
 whether the user’s position is visible, use the `userLocationVisible` property.
 The default value of this property is `NO`.

 Your app must specify a value for `NSLocationWhenInUseUsageDescription` or
 `NSLocationAlwaysUsageDescription` in its `Info.plist` to satisfy the
 requirements of the underlying Core Location framework when enabling this
 property.

 If you implement a custom location manager, set the `locationManager` before
 calling `showsUserLocation`.
 */
@property (nonatomic, assign) BOOL showsUserLocation;

/**
 A boolean value indicating whether camera animation duration is set based
 on the time difference between the last location update and the current one
 or the default animation duration of 1 second.

 The default value of this property is `NO`
 */
@property (nonatomic, assign) BOOL dynamicNavigationCameraAnimationDuration;

/**
 A Boolean value indicating whether the map may request authorization to use location services.

 Setting this property to `YES` causes the map view to use the Core Location
 framework to request authorization when authorizationStatus == kCLAuthorizationStatusNotDetermined.

 The default value of this property is `YES`.
 */
@property (nonatomic, assign) BOOL shouldRequestAuthorizationToUseLocationServices;

/**
 A Boolean value indicating whether the device’s current location is visible in
 the map view.

 Use `showsUserLocation` to control the visibility of the on-screen user
 location annotation.
 */
@property (nonatomic, assign, readonly, getter=isUserLocationVisible) BOOL userLocationVisible;

/**
 Returns the annotation object indicating the user’s current location.
 */
@property (nonatomic, readonly, nullable) MLNUserLocation *userLocation;

/**
 The mode used to track the user location. The default value is
 ``MLNUserTrackingMode/MLNUserTrackingModeNone``.

 Changing the value of this property updates the map view with an animated
 transition. If you don’t want to animate the change, use the
 `-setUserTrackingMode:animated:` method instead.

 #### Related examples
 - TODO: Customize the user location annotation and learn how to customize the
 default user location annotation shown by ``MLNUserTrackingMode``.
 */
@property (nonatomic, assign) MLNUserTrackingMode userTrackingMode;

/**
 Deprecated. Sets the mode used to track the user location, with an optional transition.

 To specify a completion handler to execute after the animation finishes, use
 the `-setUserTrackingMode:animated:completionHandler:` method.

 @param mode The mode used to track the user location.
 @param animated If `YES`, there is an animated transition from the current
    viewport to a viewport that results from the change to `mode`. If `NO`, the
    map view instantaneously changes to the new viewport. This parameter only
    affects the initial transition; subsequent changes to the user location or
    heading are always animated.
 */
- (void)setUserTrackingMode:(MLNUserTrackingMode)mode
                   animated:(BOOL)animated
    __attribute__((deprecated("Use `-setUserTrackingMode:animated:completionHandler:` instead.")));

/**
 Sets the mode used to track the user location, with an optional transition and
 completion handler.

 @param mode The mode used to track the user location.
 @param animated If `YES`, there is an animated transition from the current
    viewport to a viewport that results from the change to `mode`. If `NO`, the
    map view instantaneously changes to the new viewport. This parameter only
    affects the initial transition; subsequent changes to the user location or
    heading are always animated.
 @param completion The block executed after the animation finishes.
 */
- (void)setUserTrackingMode:(MLNUserTrackingMode)mode
                   animated:(BOOL)animated
          completionHandler:(nullable void (^)(void))completion;

/**
 The vertical alignment of the user location annotation within the receiver. The
 default value is ``MLNAnnotationVerticalAlignment/MLNAnnotationVerticalAlignmentCenter``.

 Changing the value of this property updates the map view with an animated
 transition. If you don’t want to animate the change, use the
 `-setUserLocationVerticalAlignment:animated:` method instead.
 */
@property (nonatomic, assign) MLNAnnotationVerticalAlignment userLocationVerticalAlignment
    __attribute__((
        deprecated("Use ``MLNMapViewDelegate/mapViewUserLocationAnchorPoint:`` instead.")));

/**
 Sets the vertical alignment of the user location annotation within the
 receiver, with an optional transition.

 @param alignment The vertical alignment of the user location annotation.
 @param animated If `YES`, the user location annotation animates to its new
    position within the map view. If `NO`, the user location annotation
    instantaneously moves to its new position.
 */
- (void)setUserLocationVerticalAlignment:(MLNAnnotationVerticalAlignment)alignment
                                animated:(BOOL)animated
    __attribute__((
        deprecated("Use ``MLNMapViewDelegate/mapViewUserLocationAnchorPoint:`` instead.")));

/**
 Updates the position of the user location annotation view by retreiving the user's last
 known location.
 */
- (void)updateUserLocationAnnotationView;

/**
 Updates the position of the user location annotation view by retreiving the user's last
 known location with a specified duration.
 @param duration The duration to animate the change in seconds.
*/
- (void)updateUserLocationAnnotationViewAnimatedWithDuration:(NSTimeInterval)duration;

/**
 A Boolean value indicating whether the user location annotation may display a
 permanent heading indicator.

 Setting this property to `YES` causes the default user location annotation to
 appear and always show an arrow-shaped heading indicator, if heading is
 available. This property does not rotate the map; for that, see
 ``MLNUserTrackingMode/MLNUserTrackingModeFollowWithHeading``.

 This property has no effect when ``userTrackingMode`` is
 ``MLNUserTrackingMode/MLNUserTrackingModeFollowWithHeading`` or
 ``MLNUserTrackingMode/MLNUserTrackingModeFollowWithCourse``.

 The default value of this property is `NO`.
 */
@property (nonatomic, assign) BOOL showsUserHeadingIndicator;

/**
 Whether the map view should display a heading calibration alert when necessary.
 The default value is `YES`.
 */
@property (nonatomic, assign) BOOL displayHeadingCalibration;

/**
 The geographic coordinate that is the subject of observation as the user
 location is being tracked.

 By default, this property is set to an invalid coordinate, indicating that
 there is no target. In course tracking mode, the target forms one of two foci
 in the viewport, the other being the user location annotation. Typically, this
 property is set to a destination or waypoint in a real-time navigation scene.
 As the user annotation moves toward the target, the map automatically zooms in
 to fit both foci optimally within the viewport.

 This property has no effect if the `userTrackingMode` property is set to a
 value other than ``MLNUserTrackingMode/MLNUserTrackingModeFollowWithCourse``.

 Changing the value of this property updates the map view with an animated
 transition. If you don’t want to animate the change, use the
 `-setTargetCoordinate:animated:` method instead.
 */
@property (nonatomic, assign) CLLocationCoordinate2D targetCoordinate;

/**
 Deprecated. Sets the geographic coordinate that is the subject of observation as
 the user location is being tracked, with an optional transition animation.

 By default, the target coordinate is set to an invalid coordinate, indicating
 that there is no target. In course tracking mode, the target forms one of two
 foci in the viewport, the other being the user location annotation. Typically,
 the target is set to a destination or waypoint in a real-time navigation scene.
 As the user annotation moves toward the target, the map automatically zooms in
 to fit both foci optimally within the viewport.

 This method has no effect if the `userTrackingMode` property is set to a value
 other than ``MLNUserTrackingMode/MLNUserTrackingModeFollowWithCourse``.

 To specify a completion handler to execute after the animation finishes, use
 the `-setTargetCoordinate:animated:completionHandler:` method.

 @param targetCoordinate The target coordinate to fit within the viewport.
 @param animated If `YES`, the map animates to fit the target within the map
    view. If `NO`, the map fits the target instantaneously.
 */
- (void)setTargetCoordinate:(CLLocationCoordinate2D)targetCoordinate
                   animated:(BOOL)animated
    __attribute__((deprecated("Use `-setTargetCoordinate:animated:completionHandler:` instead.")));

/**
 Sets the geographic coordinate that is the subject of observation as the user
 location is being tracked, with an optional transition animation and completion
 handler.

 By default, the target coordinate is set to an invalid coordinate, indicating
 that there is no target. In course tracking mode, the target forms one of two
 foci in the viewport, the other being the user location annotation. Typically,
 the target is set to a destination or waypoint in a real-time navigation scene.
 As the user annotation moves toward the target, the map automatically zooms in
 to fit both foci optimally within the viewport.

 This method has no effect if the `userTrackingMode` property is set to a value
 other than ``MLNUserTrackingMode/MLNUserTrackingModeFollowWithCourse``.

 @param targetCoordinate The target coordinate to fit within the viewport.
 @param animated If `YES`, the map animates to fit the target within the map
    view. If `NO`, the map fits the target instantaneously.
 @param completion The block executed after the animation finishes.
 */
- (void)setTargetCoordinate:(CLLocationCoordinate2D)targetCoordinate
                   animated:(BOOL)animated
          completionHandler:(nullable void (^)(void))completion;

// MARK: Configuring How the User Interacts with the Map

/**
 A Boolean value that determines whether the user may zoom the map in and
 out, changing the zoom level.

 When this property is set to `YES`, the default, the user may zoom the map
 in and out by pinching two fingers or by double tapping, holding, and moving
 the finger up and down.

 This property controls only user interactions with the map. If you set the
 value of this property to `NO`, you may still change the map zoom
 programmatically.
 */
@property (nonatomic, getter=isZoomEnabled) BOOL zoomEnabled;

/**
 A boolean value that reverses the direction of the quick zoom gesture.

 When this property is set, the zoom-in and zoom-out behavior during the quick
 zoom gesture (also called one-finger zoom) is reversed, aligning with the
 behavior in Apple Maps. The default value is `NO`.
 */
@property (nonatomic, getter=isQuickZoomReversed) BOOL quickZoomReversed;

/**
 A Boolean value that determines whether the user may scroll around the map,
 changing the center coordinate.

 When this property is set to `YES`, the default, the user may scroll the map
 by dragging or swiping with one finger.

 This property controls only user interactions with the map. If you set the
 value of this property to `NO`, you may still change the map location
 programmatically.
 */
@property (nonatomic, getter=isScrollEnabled) BOOL scrollEnabled;

/**
 The scrolling mode the user is allowed to use to interact with the map.

`MLNPanScrollingModeHorizontal` only allows the user to scroll horizontally on the map,
 restricting a user's ability to scroll vertically.
`MLNPanScrollingModeVertical` only allows the user to scroll vertically on the map,
 restricting a user's ability to scroll horizontally.
 ``MLNPanScrollingMode/MLNPanScrollingModeDefault`` allows the user to scroll both horizontally and
vertically on the map.

 By default, this property is set to ``MLNPanScrollingMode/MLNPanScrollingModeDefault``.
 */
@property (nonatomic, assign) MLNPanScrollingMode panScrollingMode;

/**
 A Boolean value that determines whether the user may rotate the map,
 changing the direction.

 When this property is set to `YES`, the default, the user may rotate the map
 by moving two fingers in a circular motion.

 This property controls only user interactions with the map. If you set the
 value of this property to `NO`, you may still rotate the map
 programmatically.
 */
@property (nonatomic, getter=isRotateEnabled) BOOL rotateEnabled;

/**
 A Boolean value that determines whether the user may change the pitch (tilt) of
 the map.

 When this property is set to `YES`, the default, the user may tilt the map by
 vertically dragging two fingers.

 This property controls only user interactions with the map. If you set the
 value of this property to `NO`, you may still change the pitch of the map
 programmatically.

 The default value of this property is `YES`.
 */
@property (nonatomic, getter=isPitchEnabled) BOOL pitchEnabled;

/**
 A Boolean value that determines whether gestures are anchored to the center coordinate of the map
 while rotating or zooming. Default value is set to NO.
 */
@property (nonatomic) BOOL anchorRotateOrZoomGesturesToCenterCoordinate;

/**
 A Boolean value that determines whether the user will receive haptic feedback
 for certain interactions with the map.

 When this property is set to `YES`, the default, a `UIImpactFeedbackStyleLight`
 haptic feedback event be played when the user rotates the map to due north
 (0°).

 This feature requires a device that supports haptic feedback, running iOS 10 or
 newer.
 */
@property (nonatomic, getter=isHapticFeedbackEnabled) BOOL hapticFeedbackEnabled;

/**
 A floating-point value that determines the rate of deceleration after the user
 lifts their finger.

 Your application can use the ``MLNMapViewDecelerationRateNormal`` and
 ``MLNMapViewDecelerationRateFast`` constants as reference points for reasonable
 deceleration rates. ``MLNMapViewDecelerationRateImmediate`` can be used to
 disable deceleration entirely.
 */
@property (nonatomic) CGFloat decelerationRate;

// MARK: Manipulating the Viewpoint

/**
 The geographic coordinate at the center of the map view.

 Changing the value of this property centers the map on the new coordinate
 without changing the current zoom level.

 Changing the value of this property updates the map view immediately. If you
 want to animate the change, use the `-setCenterCoordinate:animated:` method
 instead.
 */
@property (nonatomic) CLLocationCoordinate2D centerCoordinate;

/**
 Changes the center coordinate of the map and optionally animates the change.

 Changing the center coordinate centers the map on the new coordinate without
 changing the current zoom level. For animated changes, wait until the map view has
 finished loading before calling this method.

 @param coordinate The new center coordinate for the map.
 @param animated Specify `YES` if you want the map view to scroll to the new
    location or `NO` if you want the map to display the new location
    immediately.

 > Note: The behavior of this method is undefined if called in response to
 `UIApplicationWillTerminateNotification`.
 */
- (void)setCenterCoordinate:(CLLocationCoordinate2D)coordinate animated:(BOOL)animated;

/**
 Changes the center coordinate and zoom level of the map and optionally animates
 the change. For animated changes, wait until the map view has
 finished loading before calling this method.

 @param centerCoordinate The new center coordinate for the map.
 @param zoomLevel The new zoom level for the map.
 @param animated Specify `YES` if you want the map view to animate scrolling and
    zooming to the new location or `NO` if you want the map to display the new
    location immediately.

 > Note: The behavior of this method is undefined if called in response to
 `UIApplicationWillTerminateNotification`.
 */
- (void)setCenterCoordinate:(CLLocationCoordinate2D)centerCoordinate
                  zoomLevel:(double)zoomLevel
                   animated:(BOOL)animated;

/**
 Changes the center coordinate, zoom level, and direction of the map and
 optionally animates the change. For animated changes, wait until the map view has
 finished loading before calling this method.

 @param centerCoordinate The new center coordinate for the map.
 @param zoomLevel The new zoom level for the map.
 @param direction The new direction for the map, measured in degrees relative to
    true north. A negative value leaves the map’s direction unchanged.
 @param animated Specify `YES` if you want the map view to animate scrolling,
    zooming, and rotating to the new location or `NO` if you want the map to
    display the new location immediately.

 > Note: The behavior of this method is undefined if called in response to
 `UIApplicationWillTerminateNotification`.
 */
- (void)setCenterCoordinate:(CLLocationCoordinate2D)centerCoordinate
                  zoomLevel:(double)zoomLevel
                  direction:(CLLocationDirection)direction
                   animated:(BOOL)animated;

/**
 Changes the center coordinate, zoom level, and direction of the map, calling a
 completion handler at the end of an optional animation. For animated changes,
 wait until the map view has finished loading before calling this method.

 @param centerCoordinate The new center coordinate for the map.
 @param zoomLevel The new zoom level for the map.
 @param direction The new direction for the map, measured in degrees relative to
    true north. A negative value leaves the map’s direction unchanged.
 @param animated Specify `YES` if you want the map view to animate scrolling,
    zooming, and rotating to the new location or `NO` if you want the map to
    display the new location immediately.
 @param completion The block executed after the animation finishes.

 > Note: The behavior of this method is undefined if called in response to
 `UIApplicationWillTerminateNotification`.
 */
- (void)setCenterCoordinate:(CLLocationCoordinate2D)centerCoordinate
                  zoomLevel:(double)zoomLevel
                  direction:(CLLocationDirection)direction
                   animated:(BOOL)animated
          completionHandler:(nullable void (^)(void))completion;

/** The zoom level of the receiver.

 In addition to affecting the visual size and detail of features on the map,
 the zoom level affects the size of the vector tiles that are loaded. At zoom
 level 0, each tile covers the entire world map; at zoom level 1, it covers ¼
 of the world; at zoom level 2, <sup>1</sup>⁄<sub>16</sub> of the world, and
 so on.

 Changing the value of this property updates the map view immediately. If you
 want to animate the change, use the `-setZoomLevel:animated:` method instead.
 */
@property (nonatomic) double zoomLevel;

/**
 Changes the zoom level of the map and optionally animates the change.

 Changing the zoom level scales the map without changing the current center
 coordinate.

 @param zoomLevel The new zoom level for the map.
 @param animated Specify `YES` if you want the map view to animate the change
    to the new zoom level or `NO` if you want the map to display the new
    zoom level immediately.
 */
- (void)setZoomLevel:(double)zoomLevel animated:(BOOL)animated;

/**
 * The minimum zoom level at which the map can be shown.
 *
 * Depending on the map view’s aspect ratio, the map view may be prevented
 * from reaching the minimum zoom level, in order to keep the map from
 * repeating within the current viewport.
 *
 * If the value of this property is greater than that of the
 * maximumZoomLevel property, the behavior is undefined.
 *
 * The default minimumZoomLevel is 0.
 */
@property (nonatomic) double minimumZoomLevel;

/**
 * The maximum zoom level the map can be shown at.
 *
 * If the value of this property is smaller than that of the
 * minimumZoomLevel property, the behavior is undefined.
 *
 * The default maximumZoomLevel is 22. The upper bound for this property
 * is 25.5.
 */
@property (nonatomic) double maximumZoomLevel;

/**
 * The maximum bounds of the map that can be shown on screen.
 *
 * @param MLNCoordinateBounds the bounds to constrain the screen to.
 */
@property (nonatomic) MLNCoordinateBounds maximumScreenBounds;

/**
 The heading of the map, measured in degrees clockwise from true north.

 The value `0` means that the top edge of the map view corresponds to true
 north. The value `90` means the top of the map is pointing due east. The
 value `180` means the top of the map points due south, and so on.

 Changing the value of this property updates the map view immediately. If you
 want to animate the change, use the `-setDirection:animated:` method instead.
 */
@property (nonatomic) CLLocationDirection direction;

/**
 Changes the heading of the map and optionally animates the change.

 @param direction The heading of the map, measured in degrees clockwise from
    true north.
 @param animated Specify `YES` if you want the map view to animate the change
    to the new heading or `NO` if you want the map to display the new
    heading immediately.

 Changing the heading rotates the map without changing the current center
 coordinate or zoom level.
 */
- (void)setDirection:(CLLocationDirection)direction animated:(BOOL)animated;

/**
 The minimum pitch of the map’s camera toward the horizon measured in degrees.

 If the value of this property is greater than that of the `maximumPitch`
 property, the behavior is undefined. The pitch may not be less than 0
 regardless of this property.

 The default value of this property is 0 degrees, allowing the map to appear
 two-dimensional.
 */
@property (nonatomic) CGFloat minimumPitch;

/**
 The maximum pitch of the map’s camera toward the horizon measured in degrees.

 If the value of this property is smaller than that of the `minimumPitch`
 property, the behavior is undefined. The pitch may not exceed 60 degrees
 regardless of this property.

 The default value of this property is 60 degrees.
 */
@property (nonatomic) CGFloat maximumPitch;

/**
 Resets the map rotation to a northern heading — a `direction` of `0` degrees.
 */
- (IBAction)resetNorth;

/**
 Resets the map to the current style’s default viewport.

 If the style doesn’t specify a default viewport, the map resets to a minimum
 zoom level, a center coordinate of (0, 0), and a northern heading.
 */
- (IBAction)resetPosition;

/**
 The coordinate bounds visible in the receiver’s viewport.

 Changing the value of this property updates the receiver immediately. If you
 want to animate the change, call `-setVisibleCoordinateBounds:animated:`
 instead.

 If a longitude is less than −180 degrees or greater than 180 degrees, the
 visible bounds straddles the antimeridian or international date line. For
 example, if both Tokyo and San Francisco are visible, the visible bounds might
 extend from (35.68476, −220.24257) to (37.78428, −122.41310).
 */
@property (nonatomic) MLNCoordinateBounds visibleCoordinateBounds;

/**
 Changes the receiver’s viewport to fit the given coordinate bounds,
 optionally animating the change.

 To bring both sides of the antimeridian or international date line into view,
 specify some longitudes less than −180 degrees or greater than 180 degrees. For
 example, to show both Tokyo and San Francisco simultaneously, you could set the
 visible bounds to extend from (35.68476, −220.24257) to (37.78428, −122.41310).

 @param bounds The bounds that the viewport will show in its entirety.
 @param animated Specify `YES` to animate the change by smoothly scrolling
    and zooming or `NO` to immediately display the given bounds.
 */
- (void)setVisibleCoordinateBounds:(MLNCoordinateBounds)bounds animated:(BOOL)animated;

/**
 Deprecated. Changes the receiver’s viewport to fit the given coordinate bounds with
 some additional padding on each side.

 To bring both sides of the antimeridian or international date line into view,
 specify some longitudes less than −180 degrees or greater than 180 degrees. For
 example, to show both Tokyo and San Francisco simultaneously, you could set the
 visible bounds to extend from (35.68476, −220.24257) to (37.78428, −122.41310).

 To specify a completion handler to execute after the animation finishes, use
 the `-setVisibleCoordinateBounds:edgePadding:animated:completionHandler:` method.

 @param bounds The bounds that the viewport will show in its entirety.
 @param insets The minimum padding (in screen points) that will be visible
    around the given coordinate bounds.
 @param animated Specify `YES` to animate the change by smoothly scrolling and
    zooming or `NO` to immediately display the given bounds.
 */
- (void)setVisibleCoordinateBounds:(MLNCoordinateBounds)bounds
                       edgePadding:(UIEdgeInsets)insets
                          animated:(BOOL)animated
    __attribute__((deprecated(
        "Use `-setVisibleCoordinateBounds:edgePadding:animated:completionHandler:` instead.")));

/**
 Changes the receiver’s viewport to fit the given coordinate bounds with some
 additional padding on each side, optionally calling a completion handler.

 To bring both sides of the antimeridian or international date line into view,
 specify some longitudes less than −180 degrees or greater than 180 degrees. For
 example, to show both Tokyo and San Francisco simultaneously, you could set the
 visible bounds to extend from (35.68476, −220.24257) to (37.78428, −122.41310).

 @param bounds The bounds that the viewport will show in its entirety.
 @param insets The minimum padding (in screen points) that will be visible
    around the given coordinate bounds.
 @param animated Specify `YES` to animate the change by smoothly scrolling and
    zooming or `NO` to immediately display the given bounds.
 @param completion The block executed after the animation finishes.
 */
- (void)setVisibleCoordinateBounds:(MLNCoordinateBounds)bounds
                       edgePadding:(UIEdgeInsets)insets
                          animated:(BOOL)animated
                 completionHandler:(nullable void (^)(void))completion;

/**
 Changes the receiver’s viewport to fit all of the given coordinates with some
 additional padding on each side.

 To bring both sides of the antimeridian or international date line into view,
 specify some longitudes less than −180 degrees or greater than 180 degrees. For
 example, to show both Tokyo and San Francisco simultaneously, you could set the
 visible coordinates to (35.68476, −220.24257) and (37.78428, −122.41310).

 @param coordinates The coordinates that the viewport will show.
 @param count The number of coordinates. This number must not be greater than
    the number of elements in `coordinates`.
 @param insets The minimum padding (in screen points) that will be visible
    around the given coordinate bounds.
 @param animated Specify `YES` to animate the change by smoothly scrolling and
    zooming or `NO` to immediately display the given bounds.
 */
- (void)setVisibleCoordinates:(const CLLocationCoordinate2D *)coordinates
                        count:(NSUInteger)count
                  edgePadding:(UIEdgeInsets)insets
                     animated:(BOOL)animated;

/**
 Changes the receiver’s viewport to fit all of the given coordinates with some
 additional padding on each side, optionally calling a completion handler.

 To bring both sides of the antimeridian or international date line into view,
 specify some longitudes less than −180 degrees or greater than 180 degrees. For
 example, to show both Tokyo and San Francisco simultaneously, you could set the
 visible coordinates to (35.68476, −220.24257) and (37.78428, −122.41310).

 @param coordinates The coordinates that the viewport will show.
 @param count The number of coordinates. This number must not be greater than
    the number of elements in `coordinates`.
 @param insets The minimum padding (in screen points) that will be visible
    around the given coordinate bounds.
 @param direction The direction to rotate the map to, measured in degrees
    relative to true north. A negative value leaves the map’s direction
    unchanged.
 @param duration The duration to animate the change in seconds.
 @param function The timing function to animate the change.
 @param completion The block executed after the animation finishes.
 */
- (void)setVisibleCoordinates:(const CLLocationCoordinate2D *)coordinates
                        count:(NSUInteger)count
                  edgePadding:(UIEdgeInsets)insets
                    direction:(CLLocationDirection)direction
                     duration:(NSTimeInterval)duration
      animationTimingFunction:(nullable CAMediaTimingFunction *)function
            completionHandler:(nullable void (^)(void))completion;

/**
 Sets the visible region so that the map displays the specified annotations.

 Calling this method updates the value in the `visibleCoordinateBounds` property
 and potentially other properties to reflect the new map region. A small amount
 of padding is reserved around the edges of the map view. To specify a different
 amount of padding, use the `-showAnnotations:edgePadding:animated:` method.

 @param annotations The annotations that you want to be visible in the map.
 @param animated `YES` if you want the map region change to be animated, or `NO`
    if you want the map to display the new region immediately without animations.
 */
- (void)showAnnotations:(NSArray<id<MLNAnnotation>> *)annotations animated:(BOOL)animated;

/**
 Deprecated. Sets the visible region so that the map displays the specified
 annotations with the specified amount of padding on each side.

 Calling this method updates the value in the `visibleCoordinateBounds` property
 and potentially other properties to reflect the new map region.

 To specify a completion handler to execute after the animation finishes, use
 the `-showAnnotations:edgePadding:animated:completionHandler:` method.

 @param annotations The annotations that you want to be visible in the map.
 @param insets The minimum padding (in screen points) around the edges of the
    map view to keep clear of annotations.
 @param animated `YES` if you want the map region change to be animated, or `NO`
    if you want the map to display the new region immediately without animations.
 */
- (void)showAnnotations:(NSArray<id<MLNAnnotation>> *)annotations
            edgePadding:(UIEdgeInsets)insets
               animated:(BOOL)animated
    __attribute__((
        deprecated("Use `-showAnnotations:edgePadding:animated:completionHandler:` instead.")));

/**
 Sets the visible region so that the map displays the specified annotations with
 the specified amount of padding on each side and an optional completion
 handler.

 Calling this method updates the value in the `visibleCoordinateBounds` property
 and potentially other properties to reflect the new map region.

 @param annotations The annotations that you want to be visible in the map.
 @param insets The minimum padding (in screen points) around the edges of the
    map view to keep clear of annotations.
 @param animated `YES` if you want the map region change to be animated, or `NO`
    if you want the map to display the new region immediately without animations.
 @param completion The block executed after the animation finishes.
 */
- (void)showAnnotations:(NSArray<id<MLNAnnotation>> *)annotations
            edgePadding:(UIEdgeInsets)insets
               animated:(BOOL)animated
      completionHandler:(nullable void (^)(void))completion;

/**
 A camera representing the current viewpoint of the map.
 */
@property (nonatomic, copy) MLNMapCamera *camera;

/**
 Moves the viewpoint to a different location with respect to the map with an
 optional transition animation. For animated changes, wait until the map view has
 finished loading before calling this method.

 @param camera The new viewpoint.
 @param animated Specify `YES` if you want the map view to animate the change to
    the new viewpoint or `NO` if you want the map to display the new viewpoint
    immediately.

 #### Related examples
 - TODO: Camera animation: learn how to trigger an animation that rotates around a central point.
 */
- (void)setCamera:(MLNMapCamera *)camera animated:(BOOL)animated;

/**
 Moves the viewpoint to a different location with respect to the map with an
 optional transition duration and timing function. For animated changes, wait
 until the map view has finished loading before calling this method.

 @param camera The new viewpoint.
 @param duration The amount of time, measured in seconds, that the transition
    animation should take. Specify `0` to jump to the new viewpoint
    instantaneously.
 @param function A timing function used for the animation. Set this parameter to
    `nil` for a transition that matches most system animations. If the duration
    is `0`, this parameter is ignored.

 #### Related examples
 - TODO: Camera animation: learn how to create a timed animation that
 rotates around a central point for a specific duration.
 */
- (void)setCamera:(MLNMapCamera *)camera
               withDuration:(NSTimeInterval)duration
    animationTimingFunction:(nullable CAMediaTimingFunction *)function;

/**
 Moves the viewpoint to a different location with respect to the map with an
 optional transition duration and timing function. For animated changes, wait
 until the map view has finished loading before calling this method.

 @param camera The new viewpoint.
 @param duration The amount of time, measured in seconds, that the transition
    animation should take. Specify `0` to jump to the new viewpoint
    instantaneously.
 @param function A timing function used for the animation. Set this parameter to
    `nil` for a transition that matches most system animations. If the duration
    is `0`, this parameter is ignored.
 @param completion The block to execute after the animation finishes.
 */
- (void)setCamera:(MLNMapCamera *)camera
               withDuration:(NSTimeInterval)duration
    animationTimingFunction:(nullable CAMediaTimingFunction *)function
          completionHandler:(nullable void (^)(void))completion;

/**
 Moves the viewpoint to a different location with respect to the map with an
 optional transition duration and timing function, and optionally some additional
 padding on each side. For animated changes, wait until the map view has
 finished loading before calling this method.

 @param camera The new viewpoint.
 @param duration The amount of time, measured in seconds, that the transition
 animation should take. Specify `0` to jump to the new viewpoint
 instantaneously.
 @param function A timing function used for the animation. Set this parameter to
 `nil` for a transition that matches most system animations. If the duration
 is `0`, this parameter is ignored.
 @param edgePadding The minimum padding (in screen points) that would be visible
 around the returned camera object if it were set as the receiver’s camera.
 @param completion The block to execute after the animation finishes.
 */
- (void)setCamera:(MLNMapCamera *)camera
               withDuration:(NSTimeInterval)duration
    animationTimingFunction:(nullable CAMediaTimingFunction *)function
                edgePadding:(UIEdgeInsets)edgePadding
          completionHandler:(nullable void (^)(void))completion;

/**
 Moves the viewpoint to a different location using a transition animation that
 evokes powered flight and a default duration based on the length of the flight
 path.

 The transition animation seamlessly incorporates zooming and panning to help
 the user find his or her bearings even after traversing a great distance.

 @param camera The new viewpoint.
 @param completion The block to execute after the animation finishes.
 */
- (void)flyToCamera:(MLNMapCamera *)camera completionHandler:(nullable void (^)(void))completion;

/**
 Moves the viewpoint to a different location using a transition animation that
 evokes powered flight and an optional transition duration.

 The transition animation seamlessly incorporates zooming and panning to help
 the user find his or her bearings even after traversing a great distance.

 @param camera The new viewpoint.
 @param duration The amount of time, measured in seconds, that the transition
    animation should take. Specify `0` to jump to the new viewpoint
    instantaneously. Specify a negative value to use the default duration, which
    is based on the length of the flight path.
 @param completion The block to execute after the animation finishes.
 */
- (void)flyToCamera:(MLNMapCamera *)camera
         withDuration:(NSTimeInterval)duration
    completionHandler:(nullable void (^)(void))completion;

/**
 Moves the viewpoint to a different location using a transition animation that
 evokes powered flight and an optional transition duration and peak altitude.

 The transition animation seamlessly incorporates zooming and panning to help
 the user find his or her bearings even after traversing a great distance.

 @param camera The new viewpoint.
 @param duration The amount of time, measured in seconds, that the transition
    animation should take. Specify `0` to jump to the new viewpoint
    instantaneously. Specify a negative value to use the default duration, which
    is based on the length of the flight path.
 @param peakAltitude The altitude, measured in meters, at the midpoint of the
    animation. The value of this parameter is ignored if it is negative or if
    the animation transition resulting from a similar call to
    `-setCamera:animated:` would have a midpoint at a higher altitude.
 @param completion The block to execute after the animation finishes.
 */
- (void)flyToCamera:(MLNMapCamera *)camera
         withDuration:(NSTimeInterval)duration
         peakAltitude:(CLLocationDistance)peakAltitude
    completionHandler:(nullable void (^)(void))completion;

/**
 Moves the viewpoint to a different location using a transition animation that
 evokes powered flight.

 The transition animation seamlessly incorporates zooming and panning to help
 the user find his or her bearings even after traversing a great distance.

 @param camera The new viewpoint.
 @param insets The minimum padding (in screen points) that would be visible
    around the returned camera object if it were set as the receiver's camera.
 @param duration The amount of time, measured in seconds, that the transition
    animation should take. Specify `0` to jump to the new viewpoint
    instantaneously. Specify a negative value to use the default duration, which
    is based on the length of the flight path.
 @param completion The block to execute after the animation finishes.
 */
- (void)flyToCamera:(MLNMapCamera *)camera
          edgePadding:(UIEdgeInsets)insets
         withDuration:(NSTimeInterval)duration
    completionHandler:(nullable void (^)(void))completion;
/**
 Returns the camera that best fits the given coordinate bounds.

 @param bounds The coordinate bounds to fit to the receiver’s viewport.
 @return A camera object centered on the same location as the coordinate
    bounds with zoom level as high (close to the ground) as possible while still
    including the entire coordinate bounds. The camera object uses the current
    direction and pitch.

 > Note: The behavior of this method is undefined if called in response to
 `UIApplicationWillTerminateNotification`; you may receive a `nil` return value
 depending on the order of notification delivery.
 */
- (MLNMapCamera *)cameraThatFitsCoordinateBounds:(MLNCoordinateBounds)bounds;

/**
 Returns the camera that best fits the given coordinate bounds with some
 additional padding on each side.

 @param bounds The coordinate bounds to fit to the receiver’s viewport.
 @param insets The minimum padding (in screen points) that would be visible
    around the returned camera object if it were set as the receiver’s camera.
 @return A camera object centered on the same location as the coordinate bounds
    with zoom level as high (close to the ground) as possible while still
    including the entire coordinate bounds. The camera object uses the current
    direction and pitch.

 > Note: The behavior of this method is undefined if called in response to
 `UIApplicationWillTerminateNotification`; you may receive a `nil` return value
 depending on the order of notification delivery.
 */
- (MLNMapCamera *)cameraThatFitsCoordinateBounds:(MLNCoordinateBounds)bounds
                                     edgePadding:(UIEdgeInsets)insets;

/**
 Returns the camera that best fits the given coordinate bounds with some
 additional padding on each side, matching an existing camera as much as
 possible.

 @param camera The camera that the return camera should adhere to. All values
    on this camera will be manipulated except for pitch and direction.
 @param bounds The coordinate bounds to fit to the receiver’s viewport.
 @param insets The minimum padding (in screen points) that would be visible
    around the returned camera object if it were set as the receiver’s camera.
 @return A camera object centered on the same location as the coordinate bounds
    with zoom level as high (close to the ground) as possible while still
    including the entire coordinate bounds. The initial camera's pitch and
    direction will be honored.

 > Note: The behavior of this method is undefined if called in response to
 `UIApplicationWillTerminateNotification`; you may receive a `nil` return value
 depending on the order of notification delivery.
 */
- (MLNMapCamera *)camera:(MLNMapCamera *)camera
    fittingCoordinateBounds:(MLNCoordinateBounds)bounds
                edgePadding:(UIEdgeInsets)insets;

/**
 Returns the camera that best fits the given shape with some additional padding
 on each side, matching an existing camera as much as possible.

 @param camera The camera that the return camera should adhere to. All values
    on this camera will be manipulated except for pitch and direction.
 @param shape The shape to fit to the receiver’s viewport.
 @param insets The minimum padding (in screen points) that would be visible
    around the returned camera object if it were set as the receiver’s camera.
 @return A camera object centered on the shape's center with zoom level as high
    (close to the ground) as possible while still including the entire shape.
    The initial camera's pitch and direction will be honored.

 > Note: The behavior of this method is undefined if called in response to
 `UIApplicationWillTerminateNotification`; you may receive a `nil` return value
 depending on the order of notification delivery.
 */
- (MLNMapCamera *)camera:(MLNMapCamera *)camera
            fittingShape:(MLNShape *)shape
             edgePadding:(UIEdgeInsets)insets;

/**
 Returns the camera that best fits the given shape with some additional padding
 on each side while looking in the specified direction.

 @param shape The shape to fit to the receiver’s viewport.
 @param direction The direction of the viewport, measured in degrees clockwise
    from true north.
 @param insets The minimum padding (in screen points) that would be visible
    around the returned camera object if it were set as the receiver’s camera.
 @return A camera object centered on the shape's center with zoom level as high
    (close to the ground) as possible while still including the entire shape.
    The camera object uses the current pitch.

 > Note: The behavior of this method is undefined if called in response to
 `UIApplicationWillTerminateNotification`; you may receive a `nil` return value
 depending on the order of notification delivery.
 */
- (MLNMapCamera *)cameraThatFitsShape:(MLNShape *)shape
                            direction:(CLLocationDirection)direction
                          edgePadding:(UIEdgeInsets)insets;

/**
 Returns the point in this view’s coordinate system on which to “anchor” in
 response to a user-initiated gesture.

 For example, a pinch-to-zoom gesture would anchor the map at the midpoint of
 the pinch.

 If the ``userTrackingMode`` property is not ``MLNUserTrackingMode/MLNUserTrackingModeNone``, the
 user annotation is used as the anchor point.

 Subclasses may override this method to provide specialized behavior - for
 example, anchoring on the map’s center point to provide a "locked" zooming
 mode.

 @param gesture An anchorable user gesture.
 @return The point on which to anchor in response to the gesture.
 */
- (CGPoint)anchorPointForGesture:(UIGestureRecognizer *)gesture;

/**
 The distance from the edges of the map view’s frame to the edges of the map
 view’s logical viewport.

 When the value of this property is equal to `UIEdgeInsetsZero`, viewport
 properties such as `centerCoordinate` assume a viewport that matches the map
 view’s frame. Otherwise, those properties are inset, excluding part of the
 frame from the viewport. For instance, if the only the top edge is inset, the
 map center is effectively shifted downward.

 When the map view’s superview is an instance of `UIViewController` whose
 `automaticallyAdjustsScrollViewInsets` property is `YES`, the value of this
 property may be overridden at any time.

 The usage of `automaticallyAdjustsScrollViewInsets` has been deprecated
 use the map view’s property ``MLNMapView/automaticallyAdjustsContentInset``instead.

 Changing the value of this property updates the map view immediately. If you
 want to animate the change, use the `-setContentInset:animated:completionHandler:`
 method instead.
 */
@property (nonatomic, assign) UIEdgeInsets contentInset;

/**
 The current edge insets of the current map view’s camera.

 Camera edge insets are formed as accumulation of map view's content insets
 and the edge padding passed to the method like `seCamera:...edgePadding:`,
 `setVisibleCoordinates:...edgePadding:`, `showAnnotations:...edgePadding:` etc.

 The camera edge insets influences the `centerCoordinate` of the viewport.
 This value is read-only, in order to apply paddings,  use either persistent
 `contentInset`, either transient `edgePadding` parameter of the `set...` methods.
 */
@property (nonatomic, readonly) UIEdgeInsets cameraEdgeInsets;

/**
 Deprecated. Sets the distance from the edges of the map view’s frame to the edges
 of the map view’s logical viewport with an optional transition animation.

 When the value of this property is equal to `UIEdgeInsetsZero`, viewport
 properties such as `centerCoordinate` assume a viewport that matches the map
 view’s frame. Otherwise, those properties are inset, excluding part of the
 frame from the viewport. For instance, if the only the top edge is inset, the
 map center is effectively shifted downward.

 When the map view’s superview is an instance of `UIViewController` whose
 `automaticallyAdjustsScrollViewInsets` property is `YES`, the value of this
 property may be overridden at any time.

 The usage of `automaticallyAdjustsScrollViewInsets` has been deprecated
 use the map view’s property ``MLNMapView/automaticallyAdjustsContentInset``instead.

 To specify a completion handler to execute after the animation finishes, use
 the `-setContentInset:animated:completionHandler:` method.

 @param contentInset The new values to inset the content by.
 @param animated Specify `YES` if you want the map view to animate the change to
    the content inset or `NO` if you want the map to inset the content
    immediately.
 */
- (void)setContentInset:(UIEdgeInsets)contentInset
               animated:(BOOL)animated
    __attribute__((deprecated("Use `-setContentInset:animated:completionHandler:` instead.")));

/**
 Sets the distance from the edges of the map view’s frame to the edges of the
 map view’s logical viewport with an optional transition animation and
 completion handler.

 When the value of this property is equal to `UIEdgeInsetsZero`, viewport
 properties such as `centerCoordinate` assume a viewport that matches the map
 view’s frame. Otherwise, those properties are inset, excluding part of the
 frame from the viewport. For instance, if the only the top edge is inset, the
 map center is effectively shifted downward.

 When the map view’s superview is an instance of `UIViewController` whose
 `automaticallyAdjustsScrollViewInsets` property is `YES`, the value of this
 property may be overridden at any time.

 The usage of `automaticallyAdjustsScrollViewInsets` has been deprecated
 use the map view’s property ``MLNMapView/automaticallyAdjustsContentInset``instead.

 @param contentInset The new values to inset the content by.
 @param animated Specify `YES` if you want the map view to animate the change to
    the content inset or `NO` if you want the map to inset the content
    immediately.
 @param completion The block executed after the animation finishes.
 */
- (void)setContentInset:(UIEdgeInsets)contentInset
               animated:(BOOL)animated
      completionHandler:(nullable void (^)(void))completion;

// MARK: Converting Geographic Coordinates

/**
 Converts a point in the given view’s coordinate system to a geographic
 coordinate.

 @param point The point to convert.
 @param view The view in whose coordinate system the point is expressed.
 @return The geographic coordinate at the given point.

 #### Related examples
 - TODO: Point conversion example to learn how to convert a `CGPoint` to a map coordinate.
 */
- (CLLocationCoordinate2D)convertPoint:(CGPoint)point toCoordinateFromView:(nullable UIView *)view;

/**
 Converts a geographic coordinate to a point in the given view’s coordinate
 system.

 @param coordinate The geographic coordinate to convert.
 @param view The view in whose coordinate system the returned point should be
    expressed. If this parameter is `nil`, the returned point is expressed
    in the window’s coordinate system. If `view` is not `nil`, it must
    belong to the same window as the map view.
 @return The point (in the appropriate view or window coordinate system)
    corresponding to the given geographic coordinate.

 #### Related examples
 - TODO: Point conversion: learn how to convert a map coordinate to a `CGPoint` object.
 */
- (CGPoint)convertCoordinate:(CLLocationCoordinate2D)coordinate
               toPointToView:(nullable UIView *)view;

/**
 Converts a rectangle in the given view’s coordinate system to a geographic
 bounding box.

 If the returned coordinate bounds contains a longitude is less than −180 degrees
 or greater than 180 degrees, the bounding box straddles the antimeridian or
 international date line.

 @param rect The rectangle to convert.
 @param view The view in whose coordinate system the rectangle is expressed.
 @return The geographic bounding box coextensive with the given rectangle.
 */
- (MLNCoordinateBounds)convertRect:(CGRect)rect toCoordinateBoundsFromView:(nullable UIView *)view;

/**
 Converts a geographic bounding box to a rectangle in the given view’s
 coordinate system.

 To bring both sides of the antimeridian or international date line into view,
 specify some longitudes less than −180 degrees or greater than 180 degrees. For
 example, to show both Tokyo and San Francisco simultaneously, you could set the
 visible bounds to extend from (35.68476, −220.24257) to (37.78428, −122.41310).

 @param bounds The geographic bounding box to convert.
 @param view The view in whose coordinate system the returned rectangle should
    be expressed. If this parameter is `nil`, the returned rectangle is
    expressed in the window’s coordinate system. If `view` is not `nil`, it must
    belong to the same window as the map view.
 */
- (CGRect)convertCoordinateBounds:(MLNCoordinateBounds)bounds toRectToView:(nullable UIView *)view;

/**
 Returns the distance spanned by one point in the map view’s coordinate system
 at the given latitude and current zoom level.

 The distance between points decreases as the latitude approaches the poles.
 This relationship parallels the relationship between longitudinal coordinates
 at different latitudes.

 @param latitude The latitude of the geographic coordinate represented by the
    point.
 @return The distance in meters spanned by a single point.
 */
- (CLLocationDistance)metersPerPointAtLatitude:(CLLocationDegrees)latitude;

/**
 Returns the new map projection instance initialized with the map view,
 i.e. with the current camera state.
 */
- (MLNMapProjection *)mapProjection;

// MARK: Annotating the Map

/**
 The complete list of annotations associated with the receiver. (read-only)

 The objects in this array must adopt the ``MLNAnnotation`` protocol. If no
 annotations are associated with the map view, the value of this property is
 `nil`.
 */
@property (nonatomic, readonly, nullable) NSArray<id<MLNAnnotation>> *annotations;

/**
 Adds an annotation to the map view.

 > Note: ``MLNMultiPolyline``, ``MLNMultiPolyline``, ``MLNMultiPolyline``, and
    ``MLNPointCollection`` objects cannot be added to the map view at this time.
    Any multipoint, multipolyline, multipolygon, shape or point collection
    object that is specified is silently ignored.

 @param annotation The annotation object to add to the receiver. This object
    must conform to the ``MLNAnnotation`` protocol. The map view retains the
    annotation object.

 #### Related examples
 - TODO: add a line annotation from GeoJSON.
 - TODO: add an annotation to an ``MLNMapView`` object.
 */
- (void)addAnnotation:(id<MLNAnnotation>)annotation;

/**
 Adds an array of annotations to the map view.

 > Note: ``MLNMultiPolyline``, ``MLNMultiPolyline``, and ``MLNMultiPolyline`` objects
    cannot be added to the map view at this time. Nor can ``MLNMultiPoint``
    objects that are not instances of ``MLNPolyline`` or ``MLNPolyline``. Any
    multipoint, multipolyline, multipolygon, or shape collection objects that
    are specified are silently ignored.

 @param annotations An array of annotation objects. Each object in the array
    must conform to the ``MLNAnnotation`` protocol. The map view retains each
    individual annotation object.
 */
- (void)addAnnotations:(NSArray<id<MLNAnnotation>> *)annotations;

/**
 Removes an annotation from the map view, deselecting it if it is selected.

 Removing an annotation object dissociates it from the map view entirely,
 preventing it from being displayed on the map. Thus you would typically call
 this method only when you want to hide or delete a given annotation.

 @param annotation The annotation object to remove. This object must conform
    to the ``MLNAnnotation`` protocol
 */
- (void)removeAnnotation:(id<MLNAnnotation>)annotation;

/**
 Removes an array of annotations from the map view, deselecting any selected
 annotations in the array.

 Removing annotation objects dissociates them from the map view entirely,
 preventing them from being displayed on the map. Thus you would typically
 call this method only when you want to hide or delete the given annotations.

 @param annotations The array of annotation objects to remove. Objects in the
    array must conform to the ``MLNAnnotation`` protocol.
 */
- (void)removeAnnotations:(NSArray<id<MLNAnnotation>> *)annotations;

/**
 Returns an ``MLNAnnotationView`` if the given annotation is currently associated
 with a view, otherwise nil.

 @param annotation The annotation associated with the view.
    Annotation must conform to the ``MLNAnnotation`` protocol.
 */
- (nullable MLNAnnotationView *)viewForAnnotation:(id<MLNAnnotation>)annotation;

/**
 Returns a reusable annotation image object associated with its identifier.

 For performance reasons, you should generally reuse ``MLNAnnotationImage``
 objects for identical-looking annotations in your map views. Dequeueing
 saves time and memory during performance-critical operations such as
 scrolling.

 @param identifier A string identifying the annotation image to be reused.
    This string is the same one you specify when initially returning the
    annotation image object using the `-mapView:imageForAnnotation:` method.
 @return An annotation image object with the given identifier, or `nil` if no
    such object exists in the reuse queue.

 #### Related examples
 - TODO: Add annotation views and images: learn how to most efficiently
 reuse an ``MLNAnnotationImage``.
 */
- (nullable __kindof MLNAnnotationImage *)dequeueReusableAnnotationImageWithIdentifier:
    (NSString *)identifier;

/**
 Returns a reusable annotation view object associated with its identifier.

 For performance reasons, you should generally reuse ``MLNAnnotationView``
 objects for identical-looking annotations in your map views. Dequeueing
 saves time and memory during performance-critical operations such as
 scrolling.

 @param identifier A string identifying the annotation view to be reused.
    This string is the same one you specify when initially returning the
    annotation view object using the `-mapView:viewForAnnotation:` method.
 @return An annotation view object with the given identifier, or `nil` if no
    such object exists in the reuse queue.
 */
- (nullable __kindof MLNAnnotationView *)dequeueReusableAnnotationViewWithIdentifier:
    (NSString *)identifier;

/**
 The complete list of annotations associated with the receiver that are
 currently visible.

 The objects in this array must adopt the ``MLNAnnotation`` protocol. If no
 annotations are associated with the map view or if no annotations associated
 with the map view are currently visible, the value of this property is `nil`.
 */
@property (nonatomic, readonly, nullable) NSArray<id<MLNAnnotation>> *visibleAnnotations;

/**
 Returns the list of annotations associated with the receiver that intersect with
 the given rectangle.

 @param rect A rectangle expressed in the map view’s coordinate system.
 @return An array of objects that adopt the ``MLNAnnotation`` protocol or `nil` if
    no annotations associated with the map view are currently visible in the
    rectangle.
 */
- (nullable NSArray<id<MLNAnnotation>> *)visibleAnnotationsInRect:(CGRect)rect;

// MARK: Managing Annotation Selections

/**
 The currently selected annotations.

 Assigning a new array to this property selects only the first annotation in
 the array.

 If the annotation is of type ``MLNPointAnnotation`` and is offscreen, the camera
 will animate to bring the annotation and its callout just on screen. If you
 need finer control, consider using `-selectAnnotation:animated:`.

 > Note: In versions prior to `4.0.0` if the annotation was offscreen it was not
 selected.
 */
@property (nonatomic, copy) NSArray<id<MLNAnnotation>> *selectedAnnotations;

/**
 Deprecated. Selects an annotation and displays its callout view.

 The `animated` parameter determines whether the selection is animated including whether the map is
 panned to bring the annotation into view, specifically:

 | `animated` parameter | Effect |
 |------------------|--------|
 | `NO`             | The annotation is selected, and the callout is presented. However the map is
 not panned to bring the annotation or callout into view. The presentation of the callout is NOT
 animated. | | `YES`            | The annotation is selected, and the callout is presented. If the
 annotation is not visible (or is partially visible) *and* is of type ``MLNPointAnnotation``, the
 map is panned so that the annotation and its callout are brought into view. The annotation is *not*
 centered within the viewport. |

 Note that a selection initiated by a single tap gesture is always animated.

 To specify a completion handler to execute after the animation finishes, use
 the `-selectAnnotation:animated:completionHandler:` method.

 @param annotation The annotation object to select.
 @param animated If `YES`, the annotation and callout view are animated on-screen.

 > Note: In versions prior to `4.0.0` selecting an offscreen annotation did not
 change the camera.
 */
- (void)selectAnnotation:(id<MLNAnnotation>)annotation
                animated:(BOOL)animated
    __attribute__((deprecated("Use `-selectAnnotation:animated:completionHandler:` instead.")));

/**
 Selects an annotation and displays its callout view with an optional completion
 handler.

 The `animated` parameter determines whether the selection is animated including whether the map is
 panned to bring the annotation into view, specifically:

 | `animated` parameter | Effect |
 |------------------|--------|
 | `NO`             | The annotation is selected, and the callout is presented. However the map is
 not panned to bring the annotation or callout into view. The presentation of the callout is NOT
 animated. | | `YES`            | The annotation is selected, and the callout is presented. If the
 annotation is not visible (or is partially visible) *and* is of type ``MLNPointAnnotation``, the
 map is panned so that the annotation and its callout are brought into view. The annotation is *not*
 centered within the viewport. |

 Note that a selection initiated by a single tap gesture is always animated.

 @param annotation The annotation object to select.
 @param animated If `YES`, the annotation and callout view are animated on-screen.
 @param completion The block executed after the animation finishes.

 > Note: In versions prior to `4.0.0` selecting an offscreen annotation did not
 change the camera.
 */
- (void)selectAnnotation:(id<MLNAnnotation>)annotation
                animated:(BOOL)animated
       completionHandler:(nullable void (^)(void))completion;

/**
 :nodoc:
 Selects an annotation and displays its callout view with an optional completion
 handler. This method should be considered "alpha" and as such is subject to
 change.

 @param annotation The annotation object to select.
 @param moveIntoView If the annotation is not visible (or is partially visible) *and* is of type
 ``MLNPointAnnotation``, the map is panned so that the annotation and its callout are brought into
 view. The annotation is *not* centered within the viewport.
 @param animateSelection If `YES`, the annotation's selection state and callout view's presentation
 are animated.
 @param completion The block executed after the animation finishes.
 */
- (void)selectAnnotation:(id<MLNAnnotation>)annotation
            moveIntoView:(BOOL)moveIntoView
        animateSelection:(BOOL)animateSelection
       completionHandler:(nullable void (^)(void))completion;

/**
 Deselects an annotation and hides its callout view.

 @param annotation The annotation object to deselect.
 @param animated If `YES`, the callout view is animated offscreen.
 */
- (void)deselectAnnotation:(nullable id<MLNAnnotation>)annotation animated:(BOOL)animated;

// MARK: Overlaying the Map

/**
 The complete list of overlays associated with the receiver. (read-only)

 The objects in this array must adopt the ``MLNOverlay`` protocol. If no
 overlays are associated with the map view, the value of this property is
 empty array.
 */
@property (nonatomic, readonly, nonnull) NSArray<id<MLNOverlay>> *overlays;

/**
 Adds a single overlay object to the map.

 To remove an overlay from a map, use the `-removeOverlay:` method.

 @param overlay The overlay object to add. This object must conform to the
    ``MLNOverlay`` protocol. */
- (void)addOverlay:(id<MLNOverlay>)overlay;

/**
 Adds an array of overlay objects to the map.

 To remove multiple overlays from a map, use the `-removeOverlays:` method.

 @param overlays An array of objects, each of which must conform to the
    ``MLNOverlay`` protocol.
 */
- (void)addOverlays:(NSArray<id<MLNOverlay>> *)overlays;

/**
 Removes a single overlay object from the map.

 If the specified overlay is not currently associated with the map view, this
 method does nothing.

 @param overlay The overlay object to remove.
 */
- (void)removeOverlay:(id<MLNOverlay>)overlay;

/**
 Removes one or more overlay objects from the map.

 If a given overlay object is not associated with the map view, it is ignored.

 @param overlays An array of objects, each of which conforms to the ``MLNOverlay``
    protocol.
 */
- (void)removeOverlays:(NSArray<id<MLNOverlay>> *)overlays;

// MARK: Accessing the Underlying Map Data

/**
 Returns an array of rendered map features that intersect with a given point.

 This method may return features from any of the map’s style layers. To restrict
 the search to a particular layer or layers, use the
 `-visibleFeaturesAtPoint:inStyleLayersWithIdentifiers:` method. For more
 information about searching for map features, see that method’s documentation.

 @param point A point expressed in the map view’s coordinate system.
 @return An array of objects conforming to the ``MLNFeature`` protocol that
    represent features in the sources used by the current style.

 #### Related examples
 - TODO: Select a feature within a layer: to learn how to query an
 ``MLNMapView`` object for visible ``MLNMapView`` objects.
 */
- (NSArray<id<MLNFeature>> *)visibleFeaturesAtPoint:(CGPoint)point
    NS_SWIFT_NAME(visibleFeatures(at:));

/**
 Returns an array of rendered map features that intersect with a given point,
 restricted to the given style layers.

 This method returns all the intersecting features from the specified layers. To
 filter the returned features, use the
 `-visibleFeaturesAtPoint:inStyleLayersWithIdentifiers:predicate:` method. For
 more information about searching for map features, see that method’s
 documentation.

 @param point A point expressed in the map view’s coordinate system.
 @param styleLayerIdentifiers A set of strings that correspond to the names
    of layers defined in the current style. Only the features contained in
    these layers are included in the returned array.
 @return An array of objects conforming to the ``MLNFeature`` protocol that
    represent features in the sources used by the current style.
 */
- (NSArray<id<MLNFeature>> *)visibleFeaturesAtPoint:(CGPoint)point
                       inStyleLayersWithIdentifiers:
                           (nullable NSSet<NSString *> *)styleLayerIdentifiers
    NS_SWIFT_NAME(visibleFeatures(at:styleLayerIdentifiers:));

/**
 Returns an array of rendered map features that intersect with a given point,
 restricted to the given style layers and filtered by the given predicate.

 Each object in the returned array represents a feature rendered by the
 current style and provides access to attributes specified by the relevant map
 content sources. The returned array includes features loaded by
 ``MLNShapeSource`` and ``MLNShapeSource`` objects but does not include
 anything from ``MLNRasterTileSource`` objects, or from video or canvas sources,
 which are unsupported by this SDK.

 The returned features are drawn by a style layer in the current style. For
 example, suppose the current style uses the
 <a href="https://www.mapbox.com/vector-tiles/mapbox-streets/">Mapbox Streets source</a>,
 but none of the specified style layers includes features that have the `maki`
 property set to `bus`. If you pass a point corresponding to the location of a
 bus stop into this method, the bus stop feature does not appear in the
 resulting array. On the other hand, if the style does include bus stops, an
 ``MLNFeature`` object representing that bus stop is returned and its
 `featureAttributes` dictionary has the `maki` key set to `bus` (along with
 other attributes). The dictionary contains only the attributes provided by the
 tile source; it does not include computed attribute values or rules about how
 the feature is rendered by the current style.

 The returned array is sorted by z-order, starting with the topmost rendered
 feature and ending with the bottommost rendered feature. A feature that is
 rendered multiple times due to wrapping across the antimeridian at low zoom
 levels is included only once, subject to the caveat that follows.

 Features come from tiled vector data or GeoJSON data that is converted to tiles
 internally, so feature geometries are clipped at tile boundaries and features
 may appear duplicated across tiles. For example, suppose the specified point
 lies along a road that spans the screen. The resulting array includes those
 parts of the road that lie within the map tile that contain the specified
 point, even if the road extends into other tiles.

 To find out the layer names in a particular style, view the style in
 <a href="https://maplibre.org/maputnik">Maputnik</a>.

 Only visible features are returned. To obtain features regardless of
 visibility, use the
 ``MLNVectorTileSource/featuresInSourceLayersWithIdentifiers:predicate:`` and
 ``MLNShapeSource/featuresMatchingPredicate:`` methods on the relevant sources.

 The returned features may also include features corresponding to annotations.
 These features are not object-equal to the ``MLNAnnotation`` objects that were
 originally added to the map. To query the map for annotations, use
 `visibleAnnotations` or ``MLNMapView/visibleAnnotationsInRect:``.


 @param point A point expressed in the map view’s coordinate system.
 @param styleLayerIdentifiers A set of strings that correspond to the names of
    layers defined in the current style. Only the features contained in these
    layers are included in the returned array.
 @param predicate A predicate to filter the returned features.
 @return An array of objects conforming to the ``MLNFeature`` protocol that
    represent features in the sources used by the current style.
 */
- (NSArray<id<MLNFeature>> *)visibleFeaturesAtPoint:(CGPoint)point
                       inStyleLayersWithIdentifiers:
                           (nullable NSSet<NSString *> *)styleLayerIdentifiers
                                          predicate:(nullable NSPredicate *)predicate
    NS_SWIFT_NAME(visibleFeatures(at:styleLayerIdentifiers:predicate:));

/**
 Returns an array of rendered map features that intersect with the given
 rectangle.

 This method may return features from any of the map’s style layers. To restrict
 the search to a particular layer or layers, use the
 `-visibleFeaturesAtPoint:inStyleLayersWithIdentifiers:` method. For more
 information about searching for map features, see that method’s documentation.

 @param rect A rectangle expressed in the map view’s coordinate system.
 @return An array of objects conforming to the ``MLNFeature`` protocol that
    represent features in the sources used by the current style.
 */
- (NSArray<id<MLNFeature>> *)visibleFeaturesInRect:(CGRect)rect NS_SWIFT_NAME(visibleFeatures(in:));

/**
 Returns an array of rendered map features that intersect with the given
 rectangle, restricted to the given style layers.

 This method returns all the intersecting features from the specified layers. To
 filter the returned features, use the
 `-visibleFeaturesAtPoint:inStyleLayersWithIdentifiers:predicate:` method. For
 more information about searching for map features, see that method’s
 documentation.

 @param rect A rectangle expressed in the map view’s coordinate system.
 @param styleLayerIdentifiers A set of strings that correspond to the names of
    layers defined in the current style. Only the features contained in these
    layers are included in the returned array.
 @return An array of objects conforming to the ``MLNFeature`` protocol that
    represent features in the sources used by the current style.
 */
- (NSArray<id<MLNFeature>> *)visibleFeaturesInRect:(CGRect)rect
                      inStyleLayersWithIdentifiers:
                          (nullable NSSet<NSString *> *)styleLayerIdentifiers
    NS_SWIFT_NAME(visibleFeatures(in:styleLayerIdentifiers:));

/**
 Returns an array of rendered map features that intersect with the given
 rectangle, restricted to the given style layers and filtered by the given
 predicate.

 Each object in the returned array represents a feature rendered by the
 current style and provides access to attributes specified by the relevant map
 content sources. The returned array includes features loaded by
 ``MLNShapeSource`` and ``MLNShapeSource`` objects but does not include
 anything from ``MLNRasterTileSource`` objects, or from video or canvas sources,
 which are unsupported by this SDK.

 The returned features are drawn by a style layer in the current style. For
 example, suppose the current style uses a particular source,
 but none of the specified style layers includes features that have the `maki`
 property set to `bus`. If you pass a rectangle containing the location of a bus
 stop into this method, the bus stop feature does not appear in the resulting
 array. On the other hand, if the style does include bus stops, an ``MLNFeature``
 object representing that bus stop is returned and its `featureAttributes`
 dictionary has the `maki` key set to `bus` (along with other attributes). The
 dictionary contains only the attributes provided by the tile source; it does
 not include computed attribute values or rules about how the feature is
 rendered by the current style.

 The returned array is sorted by z-order, starting with the topmost rendered
 feature and ending with the bottommost rendered feature. A feature that is
 rendered multiple times due to wrapping across the antimeridian at low zoom
 levels is included only once, subject to the caveat that follows.

 Features come from tiled vector data or GeoJSON data that is converted to tiles
 internally, so feature geometries are clipped at tile boundaries and features
 may appear duplicated across tiles. For example, suppose the specified
 rectangle intersects with a road that spans the screen. The resulting array
 includes those parts of the road that lie within the map tiles covering the
 specified rectangle, even if the road extends into other tiles. The portion of
 the road within each map tile is included individually.

 To find out the layer names in a particular style, view the style in
 <a href="https://maplibre.org/maputnik">Maputnik</a>.

 Only visible features are returned. To obtain features regardless of
 visibility, use the
 ``MLNVectorTileSource/featuresInSourceLayersWithIdentifiers:predicate:`` and
 ``MLNShapeSource/featuresMatchingPredicate:`` methods on the relevant sources.

 @param rect A rectangle expressed in the map view’s coordinate system.
 @param styleLayerIdentifiers A set of strings that correspond to the names of
    layers defined in the current style. Only the features contained in these
    layers are included in the returned array.
 @param predicate A predicate to filter the returned features.
 @return An array of objects conforming to the ``MLNFeature`` protocol that
    represent features in the sources used by the current style.
 */
- (NSArray<id<MLNFeature>> *)visibleFeaturesInRect:(CGRect)rect
                      inStyleLayersWithIdentifiers:
                          (nullable NSSet<NSString *> *)styleLayerIdentifiers
                                         predicate:(nullable NSPredicate *)predicate
    NS_SWIFT_NAME(visibleFeatures(in:styleLayerIdentifiers:predicate:));

// MARK: Debugging the Map

/**
 The options that determine which debugging aids are shown on the map.

 These options are all disabled by default and should remain disabled in
 released software for performance and aesthetic reasons.
 */
@property (nonatomic) MLNMapDebugMaskOptions debugMask;

/**
 Returns the status of the rendering statistics overlay.
 */
- (BOOL)isRenderingStatsViewEnabled;

/**
 Enable a rendering statistics overlay with ``MLNRenderingStats`` values.
 */
- (void)enableRenderingStatsView:(BOOL)value;

/**
 Get the list of action journal log files from oldest to newest.

 @return An array of log file paths.
*/
- (NSArray<NSString *> *)getActionJournalLogFiles;

/**
 Get the action journal events from oldest to newest.

 Each element contains a serialized json object with the event data.
 Example
 `{
    "name" : "onTileAction",
    "time" : "2025-04-17T13:13:13.974Z",
    "styleName" : "Streets",
    "styleURL" : "maptiler://maps/streets",
    "event" : {
        "action" : "RequestedFromNetwork",
        "tileX" : 0,
        "tileY" : 0,
        "tileZ" : 0,
        "overscaledZ" : 0,
        "sourceID" : "openmaptiles"
    }
 }`
 */
- (NSArray<NSString *> *)getActionJournalLog;

/**
 Clear stored action journal events.
 */
- (void)clearActionJournalLog;

- (MLNBackendResource *)backendResource;

/**
 Triggers a repaint of the map.
*/
- (void)triggerRepaint;

/**
 Adds a plug-in layer that is external to this library
 */
- (void)addPluginLayerType:(Class)pluginLayerClass;

@end

NS_ASSUME_NONNULL_END
