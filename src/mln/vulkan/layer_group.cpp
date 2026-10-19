#include <mln/vulkan/layer_group.hpp>

#include <mln/gfx/drawable_tweaker.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/upload_pass.hpp>
#include <mln/vulkan/context.hpp>
#include <mln/vulkan/drawable.hpp>
#include <mln/vulkan/render_pass.hpp>
#include <mln/vulkan/command_encoder.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/util/convert.hpp>

namespace mln {
namespace vulkan {

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mln::LayerGroup(layerIndex_, initialCapacity, std::move(name_)),
      uniformBuffers(DescriptorSetType::Layer,
                     shaders::layerSSBOStartId,
                     shaders::maxSSBOCountPerLayer,
                     shaders::maxUBOCountPerLayer) {}

void LayerGroup::upload(gfx::UploadPass& uploadPass) {
    if (!enabled) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = uploadPass.createDebugGroup(getName() + "-upload");
#endif

    visitDrawables([&](gfx::Drawable& drawable_) {
        if (drawable_.getEnabled()) {
            auto& drawable = static_cast<Drawable&>(drawable_);
            drawable.upload(uploadPass);
        }
    });
}

void LayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);
    auto& encoder = renderPass.getEncoder();

    bool bindUBOs = false;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        if (!bindUBOs) {
            uniformBuffers.bindDescriptorSets(encoder);
            bindUBOs = true;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        drawable.draw(parameters);
    });
}

} // namespace vulkan
} // namespace mln
