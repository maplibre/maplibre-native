
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>

#include <mbgl/webgpu/layer_group.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/webgpu/context.hpp>

#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

#include <mbgl/util/logging.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {
namespace webgpu {

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mbgl::LayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

void LayerGroup::upload(gfx::UploadPass& uploadPass) {
    if (!enabled) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = uploadPass.createDebugGroup(getName() + "-upload");
#endif

    visitDrawables([&](gfx::Drawable& drawable) {
        if (drawable.getEnabled()) {
            auto& drawableWebGPU = static_cast<webgpu::Drawable&>(drawable);
            drawableWebGPU.upload(uploadPass);
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

    bool bindUBOs = false;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        if (!bindUBOs) {
            // In WebGPU, binding happens per-drawable through bind groups,
            // but we still call bindWebgpu once to prepare any shared state
            static_cast<webgpu::UniformBufferArray&>(uniformBuffers).bindWebgpu(renderPass);
            bindUBOs = true;
        }

        // Copy uniform buffers from layer group to drawable for WebGPU's bind groups
        auto& drawableWebGPU = static_cast<webgpu::Drawable&>(drawable);
        for (size_t i = 0; i < uniformBuffers.allocatedSize(); ++i) {
            const auto& uniformBuffer = uniformBuffers.get(i);
            if (uniformBuffer) {
                drawableWebGPU.mutableUniformBuffers().set(i, uniformBuffer);
            }
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        drawable.draw(parameters);
    });
}

} // namespace webgpu
} // namespace mbgl
