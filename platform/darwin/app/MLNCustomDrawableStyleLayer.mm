#import "MLNCustomDrawableStyleLayer.h"
#import "MLNCustomDrawableStyleLayer_Private.h"
#import "MLNStyleLayer.h"

#import "MLNStyle_Private.h"
#import "MLNStyleLayer_Private.h"
#import "MLNGeometry_Private.h"

#include <mbgl/layermanager/layer_factory.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/util/constants.hpp>

#include <memory>
#include <cmath>

class MLNCustomDrawableLayerHost;

namespace mbgl {
    namespace style {
        class CustomDrawableLayer;
    }
}

@implementation MLNCustomDrawableStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier {
    auto layer = std::make_unique<mbgl::style::CustomDrawableLayer>(identifier.UTF8String,
                                                            std::make_unique<MLNCustomDrawableLayerHost>(self));
    return self = [super initWithPendingLayer:std::move(layer)];
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
        
        // if we have built our drawable(s) already, either update or skip
        if (interface.getDrawableCount())
            return;
        
        // set tile
        interface.setTileID({11, 327, 791});

        // add polylines
        using namespace mbgl;
        
        constexpr auto numLines = 6;
        Color colors[numLines] {Color::red(), Color::blue(), Color(1.f, 0.5f, 0, 0.5f), Color(1.f, 1.f, 0, 0.3f), Color::black(),  Color(1.f, 0, 1.f, 0.2f)};
        float blurs[numLines] {0.0f, 4.0f, 16.0f, 2.0f, 0.5f, 24.0f};
        float opacities[numLines] {1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f};
        float gapWidths[numLines] {0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        float offsets[numLines] {0.0f, -1.0f, 2.0f, -2.0f, 0.5f, -5.0f};
        float widths[numLines] {8.0f, 4.0f, 16.0f, 2.0f, 0.5f, 24.0f};

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
            
            // set property values
            interface.setColor(colors[index]);
            interface.setBlur(blurs[index]);
            interface.setOpacity(opacities[index]);
            interface.setGapWidth(gapWidths[index]);
            interface.setOffset(offsets[index]);
            interface.setWidth(widths[index]);
            
            // add polyline
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
