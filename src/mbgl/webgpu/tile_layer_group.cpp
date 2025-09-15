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

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mbgl::TileLayerGroup(layerIndex_, initialCapacity, std::move(name_)) {
}

void TileLayerGroup::upload(gfx::UploadPass& uploadPass) {
    if (!enabled) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = uploadPass.createDebugGroup(getName() + "-upload");
#endif

    // int count = 0;
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

    // Stencil clipping is handled by the render pipeline state
    // Match Metal's approach: bind uniform buffers to drawables

    // Render tiles
    // TileLayerGroup doesn't have getCurrentTileIDs() - just visit all drawables
    int drawableIndex = 0;
    visitDrawables([&](gfx::Drawable& drawable) {
        drawableIndex++;

        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        // Call bindWebgpu to bind uniform buffers (similar to Metal's bindMtl)
        auto& webgpuRenderPass = static_cast<webgpu::RenderPass&>(*parameters.renderPass);
        static_cast<webgpu::UniformBufferArray&>(uniformBuffers).bindWebgpu(webgpuRenderPass);

        // In WebGPU, we also copy uniform buffers to each drawable since we use bind groups
        auto& drawableWebGPU = static_cast<webgpu::Drawable&>(drawable);

        // Copy uniform buffers from layer group to drawable
        for (size_t i = 0; i < uniformBuffers.allocatedSize(); ++i) {
            const auto& uniformBuffer = uniformBuffers.get(i);
            if (uniformBuffer) {
                drawableWebGPU.mutableUniformBuffers().set(i, uniformBuffer);
            }
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
