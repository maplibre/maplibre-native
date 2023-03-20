#import "MLNMapView.h"
#import "MLNUserLocationAnnotationView.h"
#import "MLNAnnotationContainerView.h"

#include <mbgl/util/size.hpp>

namespace mbgl {
    class Map;
    class Renderer;
}

class MLNMapViewImpl;
@class MLNSource;

/// Standard animation duration for UI elements.
FOUNDATION_EXTERN const NSTimeInterval MLNAnimationDuration;

/// Minimum size of an annotation’s accessibility element.
FOUNDATION_EXTERN const CGSize MLNAnnotationAccessibilityElementMinimumSize;

/// Indicates that a method (that uses `mbgl::Map`) was called after app termination.
FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const _Nonnull MLNUnderlyingMapUnavailableException;

@interface MLNMapView (Private)

/// The map view’s OpenGL rendering context.
@property (nonatomic, readonly, nullable) EAGLContext *context;

/// Currently shown popover representing the selected annotation.
@property (nonatomic, nonnull) UIView<MLNCalloutView> *calloutViewForSelectedAnnotation;

/// Map observers
- (void)cameraWillChangeAnimated:(BOOL)animated;
- (void)cameraIsChanging;
- (void)cameraDidChangeAnimated:(BOOL)animated;
- (void)mapViewWillStartLoadingMap;
- (void)mapViewDidFinishLoadingMap;
- (void)mapViewDidFailLoadingMapWithError:(nonnull NSError *)error;
- (void)mapViewWillStartRenderingFrame;
- (void)mapViewDidFinishRenderingFrameFullyRendered:(BOOL)fullyRendered;
- (void)mapViewWillStartRenderingMap;
- (void)mapViewDidFinishRenderingMapFullyRendered:(BOOL)fullyRendered;
- (void)mapViewDidBecomeIdle;
- (void)mapViewDidFinishLoadingStyle;
- (void)sourceDidChange:(nonnull MLNSource *)source;
- (void)didFailToLoadImage:(nonnull NSString *)imageName;
- (BOOL)shouldRemoveStyleImage:(nonnull NSString *)imageName;

- (CLLocationDistance)metersPerPointAtLatitude:(CLLocationDegrees)latitude zoomLevel:(double)zoomLevel;

/** Triggers another render pass even when it is not necessary. */
- (void)setNeedsRerender;

/// Synchronously render a frame of the map.
- (BOOL)renderSync;

- (mbgl::Map &)mbglMap;
- (nonnull mbgl::Renderer *)renderer;

/** Returns whether the map view is currently loading or processing any assets required to render the map */
- (BOOL)isFullyLoaded;

/** Empties the in-memory tile cache. */
- (void)didReceiveMemoryWarning;

/** Returns an instance of MLNMapView implementation. Used for integration testing. */
- (nonnull MLNMapViewImpl *) viewImpl;

- (void)pauseRendering:(nonnull NSNotification *)notification;
- (void)resumeRendering:(nonnull NSNotification *)notification;
@property (nonatomic, nonnull) MLNUserLocationAnnotationView *userLocationAnnotationView;
@property (nonatomic, nonnull) MLNAnnotationContainerView *annotationContainerView;
@property (nonatomic, readonly) BOOL enablePresentsWithTransaction;
@property (nonatomic, assign) BOOL needsDisplayRefresh;

- (BOOL) _opaque;

@end
