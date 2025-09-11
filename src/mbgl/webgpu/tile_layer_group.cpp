#include <mbgl/webgpu/tile_layer_group.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

TileLayerGroup::TileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name)
    : mbgl::TileLayerGroup(layerIndex, initialCapacity, std::move(name)) {
}

void TileLayerGroup::upload(gfx::UploadPass& uploadPass) {
    Log::Info(Event::General, "TileLayerGroup::upload called for " + getName() + ", enabled=" + std::to_string(enabled));
    if (!enabled) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = uploadPass.createDebugGroup(getName() + "-upload");
#endif

    int count = 0;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (drawable.getEnabled()) {
            Log::Info(Event::General, "TileLayerGroup: Uploading drawable " + std::to_string(++count));
            auto& drawableWebGPU = static_cast<webgpu::Drawable&>(drawable);
            drawableWebGPU.upload(uploadPass);
        }
    });
    Log::Info(Event::General, "TileLayerGroup::upload completed, uploaded " + std::to_string(count) + " drawables");
}

void TileLayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {
    Log::Info(Event::General, "WebGPU TileLayerGroup::render called - name: " + getName() + 
              ", enabled: " + std::to_string(enabled) + 
              ", drawableCount: " + std::to_string(getDrawableCount()) +
              ", hasRenderPass: " + std::to_string(parameters.renderPass != nullptr));
    
    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        Log::Info(Event::General, "WebGPU TileLayerGroup::render early return");
        return;
    }
    
    Log::Info(Event::General, "WebGPU TileLayerGroup::render - proceeding to visit drawables for " + getName());

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // Stencil clipping is handled by the render pipeline state
    // Uniform buffers are bound per-drawable in WebGPU through bind groups
    
    // Render tiles
    // TileLayerGroup doesn't have getCurrentTileIDs() - just visit all drawables
    int drawableIndex = 0;
    visitDrawables([&](gfx::Drawable& drawable) {
        drawableIndex++;
        Log::Info(Event::General, "Visiting drawable " + std::to_string(drawableIndex) + 
                  ", enabled: " + std::to_string(drawable.getEnabled()) +
                  ", hasRenderPass: " + std::to_string(drawable.hasRenderPass(parameters.pass)));

        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        // Apply tweakers if any
        const auto& tweakers = drawable.getTweakers();
        for (const auto& tweaker : tweakers) {
            if (tweaker) {
                tweaker->execute(drawable, parameters);
            }
        }

        Log::Info(Event::General, "Calling drawable.draw() for drawable " + std::to_string(drawableIndex));
        drawable.draw(parameters);
    });
}

} // namespace webgpu
} // namespace mbgl