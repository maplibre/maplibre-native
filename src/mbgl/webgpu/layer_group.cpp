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



    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // Uniform buffers are bound per-drawable in WebGPU through bind groups

    int drawableCount = 0;
    visitDrawables([&](gfx::Drawable& drawable) {
        drawableCount++;


        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        drawable.draw(parameters);
    });


}

} // namespace webgpu
} // namespace mbgl
