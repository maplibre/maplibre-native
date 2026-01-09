#include <mbgl/mtl/tile_layer_group.hpp>

#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/drawable.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>

#include <Metal/Metal.hpp>

namespace mbgl {
namespace mtl {

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mbgl::TileLayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

void TileLayerGroup::upload(gfx::UploadPass& uploadPass) {
    if (!enabled || !getDrawableCount()) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = uploadPass.createDebugGroup(getName() + "-upload");
#endif

    visitDrawables([&](gfx::Drawable& drawable) {
        if (drawable.getEnabled()) {
            auto& drawableMTL = static_cast<Drawable&>(drawable);
            drawableMTL.upload(uploadPass);
        }
    });
}

void TileLayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }

    auto& context = static_cast<Context&>(parameters.context);
    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);
    const auto& encoder = renderPass.getMetalEncoder();
    const auto& renderable = renderPass.getDescriptor().renderable;

    // `stencilModeFor3D` uses a different stencil mask value each time its called, so if the
    // drawables in this layer use 3D stencil mode, we need to set it up here so that all the
    // drawables end up using the same mode value.
    // 2D and 3D features in the same layer group is not supported.
    bool features3d = false;
    bool stencil3d = false;
    gfx::StencilMode stencilMode3d;

    // If we're using stencil clipping, we need to handle 3D features separately
    if (stencilTiles && !stencilTiles->empty()) {
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
    // We can keep the depth-based descriptors, but the stencil-based ones can change
    // every time, as a new value is assigned in each call to `stencilModeFor3D`.
    std::optional<MTLDepthStencilStatePtr> stateStencil, stateDepthStencil;
    std::function<const MTLDepthStencilStatePtr&(bool, bool)> getDepthStencilState;
    if (features3d) {
        // If we're using group-wide states, build only the ones that actually get used
        getDepthStencilState = [&](bool depth, bool stencil) -> const MTLDepthStencilStatePtr& {
            if (depth) {
                // We assume this doesn't change over the lifetime of a layer group.
                const auto depthMode = parameters.depthModeFor3D();
                if (stencil) {
                    if (!stateDepthStencil.has_value()) {
                        stateDepthStencil = context.makeDepthStencilState(depthMode, stencilMode3d, renderable);
                    }
                    return *stateDepthStencil;
                } else {
                    if (!stateDepth) {
                        stateDepth = context.makeDepthStencilState(depthMode, gfx::StencilMode::disabled(), renderable);
                    }
                    return *stateDepth;
                }
            } else {
                if (stencil) {
                    if (!stateStencil.has_value()) {
                        stateStencil = context.makeDepthStencilState(
                            gfx::DepthMode::disabled(), stencilMode3d, renderable);
                    }
                    return *stateStencil;
                } else {
                    if (!stateNone) {
                        stateNone = context.makeDepthStencilState(
                            gfx::DepthMode::disabled(), gfx::StencilMode::disabled(), renderable);
                    }
                    return *stateNone;
                }
            }
        };

        if (stencil3d) {
            stencilMode3d = parameters.stencilModeFor3D();
            encoder->setStencilReferenceValue(stencilMode3d.ref);
        }
    } else if (stencilTiles && !stencilTiles->empty()) {
        parameters.renderTileClippingMasks(stencilTiles);
    }

    bool bindUBOs = false;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        if (!bindUBOs) {
            uniformBuffers.bindMtl(renderPass);
            bindUBOs = true;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        // For layer groups with 3D features, enable either the single-value
        // stencil mode for features with stencil enabled or disable stenciling.
        // 2D drawables will set their own stencil mode within `draw`.
        if (features3d) {
            const auto& state = getDepthStencilState(drawable.getEnableDepth(), drawable.getEnableStencil());
            renderPass.setDepthStencilState(state);
        }

        drawable.draw(parameters);
    });
}

} // namespace mtl
} // namespace mbgl
