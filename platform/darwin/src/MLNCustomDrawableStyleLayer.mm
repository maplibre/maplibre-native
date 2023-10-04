#import "MLNCustomDrawableStyleLayer.h"
#import "MLNCustomDrawableStyleLayer_Private.h"

#import "MLNMapView_Private.h"
#import "MLNStyle_Private.h"
#import "MLNStyleLayer_Private.h"
#import "MLNGeometry_Private.h"

#include <mbgl/style/layers/custom_layer.hpp>
#include <mbgl/math/wrap.hpp>

class MLNCustomDrawableLayerHost;

@interface MLNCustomDrawableStyleLayer ()

@property (nonatomic, readonly) mbgl::style::CustomLayer *rawLayer;

@property (nonatomic, readonly, nullable) MLNMapView *mapView;

@property (nonatomic, weak, readwrite) MLNStyle *style;

@end

@implementation MLNCustomDrawableStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier {
    auto layer = std::make_unique<mbgl::style::CustomLayer>(identifier.UTF8String,
                                                            std::make_unique<MLNCustomDrawableLayerHost>(self));
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

// MARK: - Adding to and removing from a map view
- (void)addToStyle:(MLNStyle *)style belowLayer:(MLNStyleLayer *)otherLayer {
    self.style = style;
    self.style.openGLLayers[self.identifier] = self;
    [super addToStyle:style belowLayer:otherLayer];
}

- (void)removeFromStyle:(MLNStyle *)style {
    [super removeFromStyle:style];
    self.style.openGLLayers[self.identifier] = nil;
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

    void render(const mbgl::style::CustomLayerRenderParameters &params) {
        if(!layer) return;

        MLNStyleLayerDrawingContext drawingContext = {
            .size = CGSizeMake(params.width, params.height),
            .centerCoordinate = CLLocationCoordinate2DMake(params.latitude, params.longitude),
            .zoomLevel = params.zoom,
            .direction = mbgl::util::wrap(params.bearing, 0., 360.),
            .pitch = static_cast<CGFloat>(params.pitch),
            .fieldOfView = static_cast<CGFloat>(params.fieldOfView),
            .projectionMatrix = MLNMatrix4Make(params.projectionMatrix)
        };
        if (layer.mapView) {
            [layer drawInMapView:layer.mapView withContext:drawingContext];
        }
    }

    void contextLost() {}

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

