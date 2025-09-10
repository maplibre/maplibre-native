#include <mbgl/webgpu/tile_layer_group.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/tile/tile_id.hpp>

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

    visitDrawables([&](gfx::Drawable& drawable) {
        if (drawable.getEnabled()) {
            auto& drawableWebGPU = static_cast<webgpu::Drawable&>(drawable);
            drawableWebGPU.upload(uploadPass);
        }
    });
}

void TileLayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // TODO: Handle stencil clipping for tiles
    // TODO: Bind uniform buffers to WebGPU pipeline
    
    // Render tiles
    for (const auto& tileID : getCurrentTileIDs()) {
        visitDrawables(parameters.pass, tileID, [&](gfx::Drawable& drawable) {
            if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
                return;
            }

            for (const auto& tweaker : drawable.getTweakers()) {
                tweaker->execute(drawable, parameters);
            }

            drawable.draw(parameters);
        });
    }
}

} // namespace webgpu
} // namespace mbgl