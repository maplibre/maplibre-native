#include <mln/gfx/drawable_custom_layer_host_tweaker.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/renderable.hpp>

#include <mln/gfx/drawable.hpp>
#include <mln/gfx/context.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mln/style/layers/mtl/custom_layer_render_parameters.hpp>
#include <mln/mtl/render_pass.hpp>
#elif MLN_RENDER_BACKEND_VULKAN
#include <mln/style/layers/vulkan/custom_layer_render_parameters.hpp>
#include <mln/vulkan/render_pass.hpp>
#endif

#include <memory>

namespace mln {
namespace gfx {

void DrawableCustomLayerHostTweaker::execute([[maybe_unused]] gfx::Drawable& drawable,
                                             mln::PaintParameters& paintParameters) {
    // custom drawing
    auto& context = paintParameters.context;
    context.resetState(paintParameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                       paintParameters.colorModeForRenderPass());

#if MLN_RENDER_BACKEND_METAL
    const auto& mtlRenderPass = static_cast<mtl::RenderPass*>(paintParameters.renderPass.get());
    mtlRenderPass->resetState();

    style::mtl::CustomLayerRenderParameters parameters(paintParameters);
#elif MLN_RENDER_BACKEND_VULKAN
    style::vulkan::CustomLayerRenderParameters parameters(paintParameters);
#else
    style::CustomLayerRenderParameters parameters(paintParameters);
#endif

    host->render(parameters);

    // Reset the view back to our original one, just in case the CustomLayer
    // changed the viewport or Framebuffer.
    paintParameters.backend.getDefaultRenderable().getResource<gfx::RenderableResource>().bind();

    context.setDirtyState();
    context.bindGlobalUniformBuffers(*paintParameters.renderPass);
}

} // namespace gfx
} // namespace mln
