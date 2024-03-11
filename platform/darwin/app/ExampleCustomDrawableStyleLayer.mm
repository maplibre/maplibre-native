#import "ExampleCustomDrawableStyleLayer.h"
#import "MLNStyleLayer.h"
#import "MLNCustomDrawableStyleLayer.h"

#import <UIKit/UIKit.h>

#include <mbgl/util/image+MLNAdditions.hpp>
#include <mbgl/layermanager/layer_factory.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/util/constants.hpp>

#include <memory>
#include <cmath>

@interface MLNCustomDrawableStyleLayer (Internal)
- (instancetype)initWithPendingLayer:(std::unique_ptr<mbgl::style::Layer>)pendingLayer;
@end

class ExampleCustomDrawableStyleLayerHost;

@implementation ExampleCustomDrawableStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier {
    auto layer = std::make_unique<mbgl::style::CustomDrawableLayer>(identifier.UTF8String,
                                                            std::make_unique<ExampleCustomDrawableStyleLayerHost>(self));
    return self = [super initWithPendingLayer:std::move(layer)];
}

@end

class ExampleCustomDrawableStyleLayerHost : public mbgl::style::CustomDrawableLayerHost {
public:
    ExampleCustomDrawableStyleLayerHost(ExampleCustomDrawableStyleLayer *styleLayer) {
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
        constexpr float extent = mbgl::util::EXTENT;

        // add polylines
        {
            using namespace mbgl;
            
            constexpr auto numLines = 6;
            Interface::LineOptions options[numLines] {
                {/*geometry=*/{},   /*blur=*/0.0f,  /*opacity=*/1.0f, /*gapWidth=*/0.0f, /*offset=*/0.0f,   /*width=*/8.0f,     /*shaderType=*/{}, /*color=*/Color::red() },
                {/*geometry=*/{},   /*blur=*/4.0f,  /*opacity=*/1.0f, /*gapWidth=*/2.0f, /*offset=*/-1.0f,  /*width=*/4.0f,     /*shaderType=*/{}, /*color=*/Color::blue() },
                {/*geometry=*/{},   /*blur=*/16.0f, /*opacity=*/1.0f, /*gapWidth=*/1.0f, /*offset=*/2.0f,   /*width=*/16.0f,    /*shaderType=*/{}, /*color=*/Color(1.f, 0.5f, 0, 0.5f) },
                {/*geometry=*/{},   /*blur=*/2.0f,  /*opacity=*/1.0f, /*gapWidth=*/1.0f, /*offset=*/-2.0f,  /*width=*/2.0f,     /*shaderType=*/{}, /*color=*/Color(1.f, 1.f, 0, 0.3f) },
                {/*geometry=*/{},   /*blur=*/0.5f,  /*opacity=*/0.5f, /*gapWidth=*/1.0f, /*offset=*/0.5f,   /*width=*/0.5f,     /*shaderType=*/{}, /*color=*/Color::black() },
                {/*geometry=*/{},   /*blur=*/24.0f, /*opacity=*/0.5f, /*gapWidth=*/1.0f, /*offset=*/-5.0f,  /*width=*/24.0f,    /*shaderType=*/{}, /*color=*/Color(1.f, 0, 1.f, 0.2f) },
            };
            for(auto& opt: options) {
                opt.geometry.beginCap = style::LineCapType::Round;
                opt.geometry.endCap = style::LineCapType::Round;
                opt.geometry.joinType = style::LineJoinType::Round;
            }
            
            constexpr auto numPoints = 100;
            GeometryCoordinates polyline;
            for (auto ipoint{0}; ipoint < numPoints; ++ipoint) {
                polyline.emplace_back(ipoint * extent / numPoints, std::sin(ipoint * 2 * M_PI / numPoints) * extent / numLines / 2.f);
            }
            
            for (auto index {0}; index <  numLines; ++index) {
                for(auto &p : polyline) {
                    p.y += extent / numLines;
                }
                
                // set property values
                interface.setLineOptions(options[index]);
                
                // add polyline
                interface.addPolyline(polyline);
            }
        }

        // add fill polygon
        {
            using namespace mbgl;

            GeometryCollection geometry{
                {
                    // ring 1
                    {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.2f)},
                    {static_cast<int16_t>(extent* 0.5f), static_cast<int16_t>(extent* 0.5f)},
                    {static_cast<int16_t>(extent* 0.7f), static_cast<int16_t>(extent* 0.5f)},
                    {static_cast<int16_t>(extent* 0.5f), static_cast<int16_t>(extent* 1.0f)},
                    {static_cast<int16_t>(extent* 0.0f), static_cast<int16_t>(extent* 0.5f)},
                    {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.2f)},
                },
                {
                    // ring 2
                    {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.25f)},
                    {static_cast<int16_t>(extent* 0.15f), static_cast<int16_t>(extent* 0.5f)},
                    {static_cast<int16_t>(extent* 0.25f), static_cast<int16_t>(extent* 0.45f)},
                    {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.25f)},
                },
            };

            // set properties
            interface.setFillOptions({/*color=*/Color::green(), /*opacity=*/0.5f});

            // add fill
            interface.addFill(geometry);
        }
        
        // add symbol
        {
            using namespace mbgl;
            GeometryCoordinate position {static_cast<int16_t>(extent* 0.5f), static_cast<int16_t>(extent* 0.5f)};
            
            // load image
            UIImage *assetImage = [UIImage imageNamed:@"pin"];
            assert(assetImage.CGImage != NULL);
            std::shared_ptr<PremultipliedImage> image = std::make_shared<PremultipliedImage>(MLNPremultipliedImageFromCGImage(assetImage.CGImage));

            // set symbol options
            Interface::SymbolOptions options;
            options.texture = interface.context.createTexture2D();
            options.texture->setImage(image);
            options.texture->setSamplerConfiguration({gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
            options.textureCoordinates = {{{0.0f, 0.08f}, {1.0f, 0.9f}}};
            const float xspan = options.textureCoordinates[1][0] - options.textureCoordinates[0][0];
            const float yspan = options.textureCoordinates[1][1] - options.textureCoordinates[0][1];
            assert(xspan > 0.0f && yspan > 0.0f);
            options.size = {static_cast<uint32_t>(image->size.width * xspan), static_cast<uint32_t>(image->size.height * yspan)};
            options.anchor = {0.5f, 0.95f};
            options.angleDegrees = 45.0f;
            options.scaleWithMap = true;
            options.pitchWithMap = false;
            interface.setSymbolOptions(options);

            // add symbol
            interface.addSymbol(position);
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
    __weak ExampleCustomDrawableStyleLayer * layerRef;
    ExampleCustomDrawableStyleLayer * layer = nil;
};
