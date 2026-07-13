#include "plugin_platform_android.hpp"
#include <mbgl/plugin/plugin_map_layer.hpp>
#include <mbgl/layermanager/layer_manager.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/plugin/plugin_style_preprocessor.hpp>
#include <mbgl/plugin/plugin_manager.hpp>
#include <mbgl/gl/command_encoder.hpp>
#include <mbgl/gl/render_pass.hpp>

using namespace mbgl::plugin;

extern "C" mbgl::plugin::RenderingContext *createPlatformRenderingContext(
    [[maybe_unused]] mbgl::PaintParameters &paintParameters) {
    RenderingContextAndroidOpenGL *renderingContext = new RenderingContextAndroidOpenGL();

    return renderingContext;
}

extern "C" {
__attribute__((used)) __attribute__((visibility("default"))) void _force_link_MapLayerTypeAndroid() {
    (void)sizeof(mbgl::plugin::PluginManager);
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
