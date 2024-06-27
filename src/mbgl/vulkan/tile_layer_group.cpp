#include <mbgl/vulkan/tile_layer_group.hpp>

#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/drawable.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace vulkan {

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mbgl::TileLayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

void TileLayerGroup::upload(gfx::UploadPass& uploadPass) {
    if (!enabled || !getDrawableCount()) {
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

void TileLayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }

    auto& context = static_cast<Context&>(parameters.context);
    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);

    // TODO 3D stencil

    // If we're doing 2D stenciling and have any drawables with tile IDs, render each tile into the stencil buffer with
    // a different value.
    if (stencilTiles && !stencilTiles->empty()) {
        parameters.renderTileClippingMasks(stencilTiles);
    }

#if !defined(NDEBUG)
    const auto debugGroupRender = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    bool bindUBOs = false;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        if (!bindUBOs) {
            bindUniformBuffers(renderPass);
            bindUBOs = true;
        }

        drawable.draw(parameters);
    });

    if (bindUBOs) {
        unbindUniformBuffers(renderPass);
    }
}

void TileLayerGroup::bindUniformBuffers(RenderPass& renderPass) const noexcept {
    for (size_t id = 0; id < uniformBuffers.allocatedSize(); id++) {
        const auto& uniformBuffer = uniformBuffers.get(id);
        if (!uniformBuffer) continue;
        const auto& buffer = static_cast<UniformBuffer&>(*uniformBuffer.get());
        const auto& resource = buffer.getBufferResource();
        renderPass.bindVertex(resource, 0, id);
        renderPass.bindFragment(resource, 0, id);
    }
}

} // namespace vulkan
} // namespace mbgl
