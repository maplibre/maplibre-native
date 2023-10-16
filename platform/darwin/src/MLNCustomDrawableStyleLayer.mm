#import "MLNCustomDrawableStyleLayer.h"
#import "MLNCustomDrawableStyleLayer_Private.h"

#import "MLNMapView_Private.h"
#import "MLNStyle_Private.h"
#import "MLNStyleLayer_Private.h"
#import "MLNGeometry_Private.h"

#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/util/constants.hpp>

#include <memory>
#include <cmath>

class MLNCustomDrawableLayerHost;

@interface MLNCustomDrawableStyleLayer ()

@property (nonatomic, readonly) mbgl::style::CustomDrawableLayer *rawLayer;

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

@end

class MLNCustomDrawableLayerHost : public mbgl::style::CustomDrawableLayerHost {
public:
    MLNCustomDrawableLayerHost(MLNCustomDrawableStyleLayer *styleLayer) {
        layerRef = styleLayer;
        layer = nil;
    }
    
    void initialize() override {
        if (layerRef == nil) return;
        else if (layer == nil) layer = layerRef;
    }
    
    void update(Interface& interface) override {
        
        // if we have build our drawable(s) already, either update or skip
        if (interface.getDrawableCount())
            return;
        
        // set tile
        interface.setTileID({11, 327, 791});

        // add polylines
        using namespace mbgl;
        
        constexpr auto numLines = 6;
        Color colors[numLines] {Color::red(), Color(1.f, 0.5f, 0, 1.f), Color(1.f, 1.f, 0, 1.f), Color::green(), Color::blue(), Color(1.f, 0, 1.f, 1.f)};
        
        constexpr auto numPoints = 100;
        GeometryCoordinates polyline;
        for (auto ipoint{0}; ipoint < numPoints; ++ipoint) {
            polyline.emplace_back(ipoint * util::EXTENT / numPoints, std::sin(ipoint * 2 * M_PI / numPoints) * util::EXTENT / numLines / 2.f);
        }
        
        gfx::PolylineGeneratorOptions options;
        options.beginCap = style::LineCapType::Round;
        options.endCap = style::LineCapType::Round;
        options.joinType = style::LineJoinType::Round;
        
        for (auto index {0}; index <  numLines; ++index) {
            for(auto &p : polyline) {
                p.y += util::EXTENT / numLines;
            }
            interface.setColor(colors[index]);
            interface.addPolyline(polyline, options);
        }
        
        // finish
        interface.finish();
    }
    
    void deinitialize() override {
        if (layer == nil) return;
        
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

