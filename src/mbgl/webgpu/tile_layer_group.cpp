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
    if (!enabled) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = uploadPass.createDebugGroup(getName() + "-upload");
#endif

    int count = 0;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (drawable.getEnabled()) {
            auto& drawableWebGPU = static_cast<webgpu::Drawable&>(drawable);
            drawableWebGPU.upload(uploadPass);
        }
    });

}

void TileLayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {

              ", hasRenderPass: " + std::to_string(parameters.renderPass != nullptr));

    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }


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

        drawable.draw(parameters);
    });
}

} // namespace webgpu
} // namespace mbgl
