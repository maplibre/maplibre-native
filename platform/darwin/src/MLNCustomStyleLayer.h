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

/// A structure containing context needed to draw a frame in an ``MLNCustomStyleLayer``.
typedef struct MLNStyleLayerDrawingContext {
  /// The size of the drawable area, in points.
  CGSize size;
  /// The center coordinate of the map view.
  CLLocationCoordinate2D centerCoordinate;
  /// The current zoom level of the map view.
  double zoomLevel;
  /// The heading (direction) in degrees clockwise from true north.
  CLLocationDirection direction;
  /// The current pitch of the map view in degrees, measured from the map plane.
  CGFloat pitch;
  /// The vertical field of view, in degrees, for the map’s perspective.
  CGFloat fieldOfView;
  /// A 4×4 matrix representing the map view’s current projection state.
  MLNMatrix4 projectionMatrix;
} MLNStyleLayerDrawingContext;

/// A style layer that is rendered by Metal code that you provide.
///
/// By default, this class does nothing. You can subclass it to provide custom
/// Metal drawing code that is run on each frame of the map.
///
/// You can access an existing ``MLNCustomStyleLayer`` using the
/// ``MLNStyle/layerWithIdentifier:`` method if you know its identifier;
/// otherwise, find it using the ``MLNStyle/layers`` property. You can also
/// create a new ``MLNCustomStyleLayer`` and add it to the style using a method such as
/// ``MLNStyle/addLayer:``.
///
/// - Warning: This API experimental. It may change
///   at any time without notice.
MLN_EXPORT
@interface MLNCustomStyleLayer : MLNStyleLayer

/// The style that currently contains the layer.
///
/// If the layer is not currently part of any style, this property is `nil`.
@property (nonatomic, weak, readonly) MLNStyle *style;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#if TARGET_OS_IPHONE
/// The OpenGL ES rendering context used for drawing this layer.
///
/// This property is only valid when using the OpenGL-based rendering backend.
/// If the Metal backend is in use, this property will be unavailable
/// (or `nil`). This property is deprecated and will be removed in a future release.
///
/// - Warning: Deprecated and will be removed in a future release.
@property (nonatomic, readonly) EAGLContext *context;
#else
/// The macOS CGL rendering context used for drawing this layer.
///
/// This property is only valid when using the OpenGL-based rendering backend.
/// If the Metal backend is in use, this property will be `NULL`.
/// This property is deprecated and may be removed in a future release.
///
/// - Warning: Deprecated and may be removed in a future release.
@property (nonatomic, readonly) CGLContextObj context;
#endif

#pragma clang diagnostic pop

#if MLN_RENDER_BACKEND_METAL
/// The Metal render command encoder used for issuing Metal draw commands.
///
/// This property is only valid when using the Metal-based rendering backend.
/// If the OpenGL backend is in use, this property will be `nil`.
@property (nonatomic, weak) id<MTLRenderCommandEncoder> renderEncoder;
#endif

/// Returns an ``MLNCustomStyleLayer`` style layer object initialized with the given identifier.
///
/// After initializing and configuring the style layer, add it to a map view’s style
/// using the ``MLNStyle/addLayer:`` or
/// ``MLNStyle/insertLayer:belowLayer:`` method.
///
/// - Parameter identifier: A string that uniquely identifies the layer in the style
///   to which it is added.
/// - Returns: An initialized custom style layer.
- (instancetype)initWithIdentifier:(NSString *)identifier;

/// Called immediately after a layer is added to a map view’s style.
///
/// Override this method in a subclass to perform any setup work before the layer
/// is used to draw a frame. For example, you might compile an OpenGL shader here.
/// The default implementation of this method does nothing.
///
/// Any resource acquired in this method must be released in
/// ``willMoveFromMapView:``.
///
/// - Parameter mapView: The map view to whose style the layer has been added.
- (void)didMoveToMapView:(MLNMapView *)mapView;

/// Called immediately before a layer is removed from a map view’s style.
///
/// Override this method in a subclass to perform any teardown work once the
/// layer has drawn its last frame and is about to be removed from the style.
/// The default implementation of this method does nothing.
///
/// This method may be called even if ``didMoveToMapView:`` has not been called.
///
/// - Parameter mapView: The map view from whose style the layer is about to be removed.
- (void)willMoveFromMapView:(MLNMapView *)mapView;

/// Called each time the layer needs to draw a new frame in a map view.
///
/// Override this method in a subclass to draw the layer’s content. The default
/// implementation of this method does nothing.
///
/// Your implementation should not make any assumptions about the OpenGL or Metal
/// state, other than that the current context/encoder is active. You may make
/// changes to the state as needed. You are not required to reset values such as
/// the depth or stencil configuration to their original values.
///
/// Be sure to draw your fragments with a *z* value of 1 to take advantage of
/// opaque fragment culling, in case the style contains any opaque layers above
/// this layer.
///
/// - Parameters:
///   - mapView: The map view to which the layer draws.
///   - context: A context structure with information defining the frame to draw.
- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context;

/// Forces the map view associated with this style to redraw the receiving layer,
/// causing the ``drawInMapView:withContext:`` method to be called again.
- (void)setNeedsDisplay;

@end

NS_ASSUME_NONNULL_END
