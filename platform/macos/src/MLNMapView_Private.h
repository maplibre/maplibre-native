#import "MLNMapView.h"
#import "MLNTileOperation.h"

#include <mbgl/util/size.hpp>

namespace mbgl {
class Map;
class Renderer;

namespace gfx {
struct RenderingStats;
}

}  // namespace mbgl

@class MLNSource;

@interface MLNMapView (Private)

/// True if the view or application is in a state where it is not expected to be
/// actively drawing.
@property (nonatomic, readonly, getter=isDormant) BOOL dormant;

// These properties exist because initially, both the latitude and longitude are
// NaN. You have to set both the latitude and longitude simultaneously. If you
// set the latitude but reuse the current longitude, and the current longitude
// happens to be NaN, there will be no change because the resulting coordinate
// pair is invalid.

/// Center latitude set independently of the center longitude in an inspectable.
@property (nonatomic) CLLocationDegrees pendingLatitude;
/// Center longitude set independently of the center latitude in an inspectable.
@property (nonatomic) CLLocationDegrees pendingLongitude;

/// The map view’s OpenGL rendering context, if it is backed by an OpenGL based view.
@property (readonly, nonatomic, nullable) CGLContextObj context;

- (mbgl::Size)framebufferSize;

/// Map observers
- (void)cameraWillChangeAnimated:(BOOL)animated;
- (void)cameraIsChanging;
- (void)cameraDidChangeAnimated:(BOOL)animated;
- (void)mapViewWillStartLoadingMap;
- (void)mapViewDidFinishLoadingMap;
- (void)mapViewDidFailLoadingMapWithError:(nonnull NSError *)error;
- (void)mapViewWillStartRenderingFrame;
- (void)mapViewDidFinishRenderingFrameFullyRendered:(BOOL)fullyRendered
                                     renderingStats:(const mbgl::gfx::RenderingStats &)stats;
- (void)mapViewWillStartRenderingMap;
- (void)mapViewDidFinishRenderingMapFullyRendered:(BOOL)fullyRendered;
- (void)mapViewDidBecomeIdle;
- (void)mapViewDidFinishLoadingStyle;
- (void)sourceDidChange:(nonnull MLNSource *)source;
- (BOOL)shouldRemoveStyleImage:(nonnull NSString *)imageName;

- (void)shaderWillCompile:(NSInteger)id
                  backend:(NSInteger)backend
                  defines:(nonnull NSString *)defines;
- (void)shaderDidCompile:(NSInteger)id
                 backend:(NSInteger)backend
                 defines:(nonnull NSString *)defines;
- (void)shaderDidFailCompile:(NSInteger)id
                     backend:(NSInteger)backend
                     defines:(nonnull NSString *)defines;
- (void)glyphsWillLoad:(nonnull NSArray<NSString *> *)fontStack range:(NSRange)range;
- (void)glyphsDidLoad:(nonnull NSArray<NSString *> *)fontStack range:(NSRange)range;
- (void)glyphsDidError:(nonnull NSArray<NSString *> *)fontStack range:(NSRange)range;
- (void)tileDidTriggerAction:(MLNTileOperation)operation
                           x:(NSInteger)x
                           y:(NSInteger)y
                           z:(NSInteger)z
                        wrap:(NSInteger)wrap
                 overscaledZ:(NSInteger)overscaledZ
                    sourceID:(nonnull NSString *)sourceID;
- (void)spriteWillLoad:(nullable NSString *)id url:(nullable NSString *)url;
- (void)spriteDidLoad:(nullable NSString *)id url:(nullable NSString *)url;
- (void)spriteDidError:(nullable NSString *)id url:(nullable NSString *)url;

/// Asynchronously render a frame of the map.
- (void)setNeedsRerender;

/// Synchronously render a frame of the map.
- (void)renderSync;

- (BOOL)isTargetingInterfaceBuilder;

- (nonnull mbgl::Renderer *)renderer;

@end
