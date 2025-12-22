#include <mbgl/interface/plugin_platform_darwin.h>
#include <mbgl/plugin/plugin_map_layer.hpp>
#include <mbgl/layermanager/layer_manager.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/plugin/plugin_style_preprocessor.hpp>

using namespace mbgl::plugin;


// This will create a rendering context for the current platform and return it.
// Implemented by the platform
extern "C" mbgl::plugin::RenderingContext *createPlatformRenderingContext(mbgl::PaintParameters& paintParameters) {
    
    const mbgl::mtl::RenderPass& renderPass = static_cast<mbgl::mtl::RenderPass&>(*paintParameters.renderPass);
    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)renderPass.getMetalEncoder().get();

    // Call render
    RenderingContextMetal *renderingContext = new RenderingContextMetal();
    renderingContext->renderEncoder = encoder;
    renderingContext->metalDevice = encoder.device;
    
    return renderingContext;
    
}


extern "C" {
__attribute__((used))
__attribute__((visibility("default")))
void _force_link_MapLayerTypeDarwin() {
    (void)sizeof(mbgl::plugin::LayerProperty);
    (void)sizeof(mbgl::plugin::MapLayerType);
    (void)sizeof(mbgl::plugin::DrawingContext);
    (void)sizeof(mbgl::plugin::RenderingContext);
    (void)sizeof(mbgl::plugin::MapLayer);
    (void)sizeof(mbgl::plugin::StylePreprocessor);

    auto lp = std::make_shared<mbgl::plugin::LayerProperty>();
    auto lt = std::make_shared<mbgl::plugin::MapLayerType>();
    auto dc = std::make_shared<mbgl::plugin::DrawingContext>();
    auto rc = std::make_shared<mbgl::plugin::RenderingContext>();
    auto l = std::make_shared<mbgl::plugin::MapLayer>();
    auto sp = std::make_shared<mbgl::plugin::StylePreprocessor>();

}
}
