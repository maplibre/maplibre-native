#include <mbgl/webgpu/layer_group.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mbgl::LayerGroup(layerIndex_, initialCapacity, std::move(name_)) {
}

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

    static int renderCallCount = 0;
    if (renderCallCount++ < 10) {
        mbgl::Log::Info(mbgl::Event::Render, "WebGPU LayerGroup::render() called for: " + getName());
    }

    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        mbgl::Log::Warning(mbgl::Event::Render, "WebGPU LayerGroup::render() early return - enabled:" +
            std::to_string(enabled) + " drawableCount:" + std::to_string(getDrawableCount()) +
            " hasRenderPass:" + std::to_string(parameters.renderPass != nullptr));
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // Match Metal's approach: bind layer group uniform buffers before drawing
    // In WebGPU, since we use bind groups per drawable, we copy buffers to each drawable

    int drawableCount = 0;
    visitDrawables([&](gfx::Drawable& drawable) {
        drawableCount++;

        static int visitCount = 0;
        if (visitCount++ < 20) {
            mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Visiting drawable #" + std::to_string(drawableCount) +
                " enabled:" + std::to_string(drawable.getEnabled()) +
                " hasRenderPass:" + std::to_string(drawable.hasRenderPass(parameters.pass)));
        }

        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        // Call bindWebgpu to bind uniform buffers (similar to Metal's bindMtl)
        auto& webgpuRenderPass = static_cast<webgpu::RenderPass&>(*parameters.renderPass);
        static_cast<webgpu::UniformBufferArray&>(uniformBuffers).bindWebgpu(webgpuRenderPass);

        // In WebGPU, we also need to copy layer group uniform buffers to each drawable
        // since WebGPU uses bind groups per drawable
        auto& drawableWebGPU = static_cast<webgpu::Drawable&>(drawable);

        // Copy uniform buffers from layer group to drawable
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

    if (drawableCount == 0) {
        mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: No drawables visited in layer group " + getName());
    }
}

} // namespace webgpu
} // namespace mbgl
