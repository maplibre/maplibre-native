#import "MLNCustomStyleLayer.h"
#import "MLNCustomStyleLayer_Private.h"

#import "MLNMapView_Private.h"
#import "MLNStyle_Private.h"
#import "MLNStyleLayer_Private.h"
#import "MLNGeometry_Private.h"

#if MLN_RENDER_BACKEND_METAL
#import <MetalKit/MetalKit.h>
#endif

#include <mbgl/style/layers/custom_layer.hpp>
#include <mbgl/math/wrap.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/style/layers/mtl/custom_layer_render_parameters.hpp>
#endif

class MLNCustomLayerHost;

@interface MLNCustomStyleLayer ()
@property (nonatomic, readonly) mbgl::style::CustomLayer *rawLayer;
@property (nonatomic, readonly, nullable) MLNMapView *mapView;
@property (nonatomic, weak, readwrite) MLNStyle *style;
@end

@implementation MLNCustomStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier {
    auto layer = std::make_unique<mbgl::style::CustomLayer>(
        identifier.UTF8String,
        std::make_unique<MLNCustomLayerHost>(self)
    );
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::CustomLayer *)rawLayer {
    return (mbgl::style::CustomLayer *)super.rawLayer;
}

- (MLNMapView *)mapView {
    if ([self.style.stylable isKindOfClass:[MLNMapView class]]) {
        return (MLNMapView *)self.style.stylable;
    }
    return nil;
}

#if TARGET_OS_IPHONE
- (EAGLContext *)context {
    return self.mapView.context;
}
#else
- (CGLContextObj)context {
    return self.mapView.context;
}
#endif

- (void)addToStyle:(MLNStyle *)style belowLayer:(MLNStyleLayer *)otherLayer {
    self.style = style;
    self.style.customLayers[self.identifier] = self;
    [super addToStyle:style belowLayer:otherLayer];
}

- (void)removeFromStyle:(MLNStyle *)style {
    [super removeFromStyle:style];
    self.style.customLayers[self.identifier] = nil;
    self.style = nil;
}

- (void)didMoveToMapView:(MLNMapView *)mapView {
}

- (void)willMoveFromMapView:(MLNMapView *)mapView {
}

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context {
}

- (void)setNeedsDisplay {
    [self.mapView setNeedsRerender];
}

@end

class MLNCustomLayerHost : public mbgl::style::CustomLayerHost {
public:
    MLNCustomLayerHost(MLNCustomStyleLayer *styleLayer) {
        layerRef = styleLayer;
        layer = nil;
    }

    void initialize() {
        if (layerRef == nil) return;
        else if (layer == nil) layer = layerRef;

        if (layer.mapView) {
            [layer didMoveToMapView:layer.mapView];
        }
    }

    void render(const mbgl::style::CustomLayerRenderParameters& parameters) {
        if (!layer) return;

#if MLN_RENDER_BACKEND_METAL
        MTL::RenderCommandEncoder* ptr =
            static_cast<const mbgl::style::mtl::CustomLayerRenderParameters&>(parameters).encoder.get();
        id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)ptr;
        layer.renderEncoder = encoder;
#endif

        MLNStyleLayerDrawingContext drawingContext = {
            .size = CGSizeMake(parameters.width, parameters.height),
            .centerCoordinate = CLLocationCoordinate2DMake(parameters.latitude, parameters.longitude),
            .zoomLevel = parameters.zoom,
            .direction = mbgl::util::wrap(parameters.bearing, 0., 360.),
            .pitch = static_cast<CGFloat>(parameters.pitch),
            .fieldOfView = static_cast<CGFloat>(parameters.fieldOfView),
            .projectionMatrix = MLNMatrix4Make(parameters.projectionMatrix),
            .nearClippedProjMatrix = MLNMatrix4Make(parameters.nearClippedProjMatrix)
        };

        if (layer.mapView) {
            [layer drawInMapView:layer.mapView withContext:drawingContext];
        }
    }

    void contextLost() {
    }

    void deinitialize() {
        if (layer == nil) return;

        if (layer.mapView) {
            [layer willMoveFromMapView:layer.mapView];
        }
        layerRef = layer;
        layer = nil;
    }

private:
    __weak MLNCustomStyleLayer * layerRef;
    MLNCustomStyleLayer * layer = nil;
};

namespace mbgl {

MLNStyleLayer* CustomStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNCustomStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
