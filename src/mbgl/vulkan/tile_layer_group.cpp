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

    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);

    // `stencilModeFor3D` uses a different stencil mask value each time its called, so if the
    // drawables in this layer use 3D stencil mode, we need to set it up here so that all the
    // drawables end up using the same mode value.
    // 2D and 3D features in the same layer group is not supported.
    bool features3d = false;
    bool stencil3d = false;
    std::optional<gfx::DepthMode> depthMode3d;
    std::optional<gfx::StencilMode> stencilMode3d;

    // If we're using stencil clipping, we need to handle 3D features separately
    if (stencilTiles && !stencilTiles->empty()) {
        // 2D and 3D features in the same layer group is not supported.
        visitDrawables([&](const gfx::Drawable& drawable) {
            if (drawable.getEnabled() && drawable.getIs3D() && drawable.hasRenderPass(parameters.pass)) {
                features3d = true;
                if (drawable.getEnableStencil()) {
                    stencil3d = true;
                }
            }
        });
    }

#if !defined(NDEBUG)
    const auto debugGroupRender = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // If we're doing 3D stenciling and have any features to draw, set up the single-value stencil mask.
    // If we're doing 2D stenciling and have any drawables with tile IDs, render each tile into the stencil buffer with
    // a different value.
    if (features3d) {
        depthMode3d = parameters.depthModeFor3D();

        if (stencil3d) {
            stencilMode3d = parameters.stencilModeFor3D();
        }
    } else if (stencilTiles && !stencilTiles->empty()) {
        parameters.renderTileClippingMasks(stencilTiles);
    }

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

        auto& drawableImpl = static_cast<Drawable&>(drawable);
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        for (size_t id = 0; id < uniformBuffers.allocatedSize(); id++) {
            const auto& uniformBuffer = uniformBuffers.get(id);
            if (!uniformBuffer) continue;

            if (features3d) {
                drawableImpl.setDepthModeFor3D(drawableImpl.getEnableDepth() ? depthMode3d.value()
                                                                             : gfx::DepthMode::disabled());

                drawableImpl.setStencilModeFor3D(drawableImpl.getEnableStencil() ? stencilMode3d.value()
                                                                                 : gfx::StencilMode::disabled());
            }

            drawableUniforms.set(id, uniformBuffer);
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
        const auto& buffer = static_cast<UniformBuffer&>(*uniformBuffer);
        const auto& resource = buffer.getBufferResource();
        renderPass.bindVertex(resource, 0, id);
        renderPass.bindFragment(resource, 0, id);
    }
}

} // namespace vulkan
} // namespace mbgl
