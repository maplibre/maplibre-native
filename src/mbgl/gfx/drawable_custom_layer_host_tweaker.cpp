#include <mbgl/gfx/drawable_custom_layer_host_tweaker.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/context.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/style/layers/mtl/custom_layer_render_parameters.hpp>
#endif

#include <memory>

namespace mbgl {
namespace gfx {

void DrawableCustomLayerHostTweaker::execute([[maybe_unused]] gfx::Drawable& drawable,
                                             const mbgl::PaintParameters& paintParameters) {
    // custom drawing
    auto& context = paintParameters.context;
    context.resetState(paintParameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                       paintParameters.colorModeForRenderPass());

#if MLN_RENDER_BACKEND_METAL
    std::unique_ptr<style::CustomLayerRenderParameters> parameters =
        std::make_unique<style::mtl::CustomLayerRenderParameters>(paintParameters);
#else
    std::unique_ptr<style::CustomLayerRenderParameters> parameters =
        std::make_unique<style::CustomLayerRenderParameters>(paintParameters);
#endif

    host->render(std::move(parameters));

    // Reset the view back to our original one, just in case the CustomLayer
    // changed the viewport or Framebuffer.
#if !MLN_RENDER_BACKEND_METAL
    paintParameters.backend.getDefaultRenderable().getResource<gfx::RenderableResource>().bind();
#endif

    context.setDirtyState();
}

} // namespace gfx
} // namespace mbgl
