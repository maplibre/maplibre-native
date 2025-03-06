#include <mbgl/vulkan/layer_group.hpp>

#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/drawable.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {
namespace vulkan {

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mbgl::LayerGroup(layerIndex_, initialCapacity, std::move(name_)),
      uniformBuffers(DescriptorSetType::Layer,
                     shaders::globalUBOCount,
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
} // namespace mbgl
