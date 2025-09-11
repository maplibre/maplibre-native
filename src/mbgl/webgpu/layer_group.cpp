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

LayerGroup::LayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name)
    : mbgl::LayerGroup(layerIndex, initialCapacity, std::move(name)) {
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
    Log::Info(Event::General, "LayerGroup::render called - name: " + getName() + 
              ", enabled: " + std::to_string(enabled) + 
              ", drawableCount: " + std::to_string(getDrawableCount()) +
              ", hasRenderPass: " + std::to_string(parameters.renderPass != nullptr));
    
    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        Log::Info(Event::General, "LayerGroup::render early return");
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // Uniform buffers are bound per-drawable in WebGPU through bind groups
    
    int drawableCount = 0;
    visitDrawables([&](gfx::Drawable& drawable) {
        drawableCount++;
        Log::Info(Event::General, "Visiting drawable " + std::to_string(drawableCount) + 
                  ", enabled: " + std::to_string(drawable.getEnabled()) +
                  ", hasRenderPass: " + std::to_string(drawable.hasRenderPass(parameters.pass)));
        
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        Log::Info(Event::General, "Calling drawable.draw()");
        drawable.draw(parameters);
    });
    
    Log::Info(Event::General, "LayerGroup::render finished, visited " + std::to_string(drawableCount) + " drawables");
}

} // namespace webgpu
} // namespace mbgl