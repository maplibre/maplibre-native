#include <mln/vulkan/tile_layer_group.hpp>

#include <mln/gfx/drawable_tweaker.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/upload_pass.hpp>
#include <mln/vulkan/context.hpp>
#include <mln/vulkan/drawable.hpp>
#include <mln/vulkan/render_pass.hpp>
#include <mln/vulkan/command_encoder.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/util/convert.hpp>
#include <mln/util/logging.hpp>

#include <unordered_map>

namespace mln {
namespace vulkan {

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mln::TileLayerGroup(layerIndex_, initialCapacity, std::move(name_)),
      uniformBuffers(DescriptorSetType::Layer,
                     shaders::globalUBOCount,
                     shaders::maxSSBOCountPerLayer,
                     shaders::maxUBOCountPerLayer) {}

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
    auto& encoder = renderPass.getEncoder();

    // `stencilModeFor3D` uses a different stencil mask value each time its called, so if the
    // drawables in this layer use 3D stencil mode, we need to set it up here so that all the
    // drawables using the SAME shader end up using the same mode value. Scoping the mode per
    // shader (rather than one shared by the whole layer group) matters when a layer group has
    // more than one shader (e.g. a plugin layer type with two shaders sharing one layer): two
    // unrelated shaders' geometry shouldn't dedup against each other's fragments, or one shader's
    // drawables can wrongly reject the other's genuinely-visible fragments purely by draw order.
    // Depth mode doesn't have this problem (no per-call-unique ref), so it stays a single value.
    // 2D and 3D features in the same layer group is not supported.
    bool features3d = false;
    std::optional<gfx::DepthMode> depthMode3d;
    std::unordered_map<const void*, gfx::StencilMode> stencilModesByShader;

    // If we're using stencil clipping, we need to handle 3D features separately
    if (stencilTiles && !stencilTiles->empty()) {
        // 2D and 3D features in the same layer group is not supported.
        visitDrawables([&](const gfx::Drawable& drawable) {
            if (drawable.getEnabled() && drawable.getIs3D() && drawable.hasRenderPass(parameters.pass)) {
                features3d = true;
            }
        });
    }

#if !defined(NDEBUG)
    const auto debugGroupRender = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // If we're doing 3D stenciling and have any features to draw, set up the depth mode (fixed
    // for the whole group) and, lazily below, a stencil mask per shader.
    // If we're doing 2D stenciling and have any drawables with tile IDs, render each tile into the stencil buffer with
    // a different value.
    if (features3d) {
        depthMode3d = parameters.depthModeFor3D();
    } else if (stencilTiles && !stencilTiles->empty()) {
        parameters.renderTileClippingMasks(stencilTiles);
    }

    bool bindUBOs = false;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        if (!bindUBOs) {
            uniformBuffers.bindDescriptorSets(encoder);
            bindUBOs = true;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        if (features3d) {
            auto& drawableImpl = static_cast<Drawable&>(drawable);

            const auto& depth = drawableImpl.getEnableDepth() ? depthMode3d.value() : gfx::DepthMode::disabled();
            drawableImpl.setDepthModeFor3D(depth);

            if (drawableImpl.getEnableStencil()) {
                const void* shaderKey = drawable.getShader().get();
                auto it = stencilModesByShader.find(shaderKey);
                if (it == stencilModesByShader.end()) {
                    it = stencilModesByShader.emplace(shaderKey, parameters.stencilModeFor3D()).first;
                }
                drawableImpl.setStencilModeFor3D(it->second);
            } else {
                drawableImpl.setStencilModeFor3D(gfx::StencilMode::disabled());
            }
        }

        drawable.draw(parameters);
    });
}

} // namespace vulkan
} // namespace mln
