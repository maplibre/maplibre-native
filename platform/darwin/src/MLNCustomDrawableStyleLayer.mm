#import "MLNCustomDrawableStyleLayer.h"
#import "MLNCustomDrawableStyleLayer_Private.h"

#import "MLNMapView_Private.h"
#import "MLNStyle_Private.h"
#import "MLNStyleLayer_Private.h"
#import "MLNGeometry_Private.h"

#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/math/wrap.hpp>

class MLNCustomDrawableLayerHost;

@interface MLNCustomDrawableStyleLayer ()

@property (nonatomic, readonly) mbgl::style::CustomDrawableLayer *rawLayer;

@property (nonatomic, readonly, nullable) MLNMapView *mapView;

@property (nonatomic, weak, readwrite) MLNStyle *style;

@end

@implementation MLNCustomDrawableStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier {
    auto layer = std::make_unique<mbgl::style::CustomDrawableLayer>(identifier.UTF8String,
                                                            std::make_unique<MLNCustomDrawableLayerHost>(self));
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::CustomDrawableLayer *)rawLayer {
    return (mbgl::style::CustomDrawableLayer *)super.rawLayer;
}

- (MLNMapView *)mapView {
    if ([self.style.stylable isKindOfClass:[MLNMapView class]]) {
        return (MLNMapView *)self.style.stylable;
    }
    return nil;
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

class MLNCustomDrawableLayerHost : public mbgl::style::CustomDrawableLayerHost {
public:
    MLNCustomDrawableLayerHost(MLNCustomDrawableStyleLayer *styleLayer) {
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

    void update(mbgl::RenderLayer& proxyLayer,
                        mbgl::gfx::ShaderRegistry& shaders,
                        mbgl::gfx::Context& context,
                        const mbgl::TransformState& state,
                        const std::shared_ptr<mbgl::UpdateParameters>&,
                        const mbgl::RenderTree& renderTree,
                        mbgl::UniqueChangeRequestVec& changes) {

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
    __weak MLNCustomDrawableStyleLayer * layerRef;
    MLNCustomDrawableStyleLayer * layer = nil;
};

namespace mbgl {

MLNStyleLayer* CustomDrawableStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNCustomDrawableStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl

