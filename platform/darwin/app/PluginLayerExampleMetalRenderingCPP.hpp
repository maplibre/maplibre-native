//
//  PluginLayerExampleMetalCPP.hpp
//  App
//
//  Created by Malcolm Toon on 12/1/25.
//

#ifndef PluginLayerExampleMetalCPP_hpp
#define PluginLayerExampleMetalCPP_hpp

#include <stdio.h>
#include <mbgl/plugin/plugin_map_layer.hpp>

#define METAL 1

#if METAL
#include <Metal/Metal.h>
#endif

namespace app {

// This is the actual layer
class MetalPluginLayer final : public mbgl::plugin::MapLayer {
public:
    // MapLayer Overrides
    void onRender(const mbgl::plugin::RenderingContext *) override;
    
    void onAddedToMap() override;
    
    void onUpdate(const mbgl::plugin::DrawingContext &) override;

    void onUpdateLayerProperties(const std::string&) override;

    void onMemoryReductionEvent() override;
    
private:
    bool _renderingResourcesCreated = false;
    mbgl::plugin::DrawingContext _lastDrawingContext;
  
    // Properties
    float _offsetX = 0;
    float _offsetY = 0;
    float _scale = 1.0;
    float _r, _g, _b, _a = 1.0;

#if METAL
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLDepthStencilState> _depthStencilStateWithoutStencil;
    void createMetalShaders(const mbgl::plugin::RenderingContext *renderingContext);
    // TODO: Need to pass this in, but where would it come from?
    MTLPixelFormat _pixelFormat = MTLPixelFormatBGRA8Unorm;
#endif
    
#if VULKAN
    
#endif
    
};

// This is effectively a definition/factory of the layer
class MetalPluginLayerType : public mbgl::plugin::MapLayerType {
public:
    // Returns the layer type string that is used in the style
    std::string getLayerType() override;

    // If this layer type requires pass 3d
    bool requiresPass3D() override;
    
    // This creates the actual map layer.  Should be overridden by the
    // implementor and return a class descended from the MapLayer below
    std::shared_ptr<mbgl::plugin::MapLayer> createMapLayer() override;
  
    // The list of properties
    std::vector<std::shared_ptr<mbgl::plugin::LayerProperty>> getLayerProperties() override;
    
};


}


#endif /* PluginLayerExampleMetalCPP_hpp */
