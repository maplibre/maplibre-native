#include <mln/mtl/tile_layer_group.hpp>

#include <mln/gfx/drawable_tweaker.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/upload_pass.hpp>
#include <mln/mtl/context.hpp>
#include <mln/mtl/drawable.hpp>
#include <mln/mtl/render_pass.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/util/convert.hpp>
#include <mln/util/logging.hpp>

#include <Metal/Metal.hpp>

#include <unordered_map>

namespace mln {
namespace mtl {

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mln::TileLayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

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
    const auto& renderable = renderPass.getDescriptor().renderable;

    // `stencilModeFor3D` uses a different stencil mask value each time its called, so if the
    // drawables in this layer use 3D stencil mode, we need to set it up here so that all the
    // drawables using the SAME shader end up using the same mode value. Scoping the mode/ref per
    // shader (rather than one shared by the whole layer group) matters when a layer group has
    // more than one shader (e.g. a plugin layer type with two shaders sharing one layer): two
    // unrelated shaders' geometry shouldn't dedup against each other's fragments, or one shader's
    // drawables can wrongly reject the other's genuinely-visible fragments purely by draw order.
    // 2D and 3D features in the same layer group is not supported.
    bool features3d = false;

    // If we're using stencil clipping, we need to handle 3D features separately
    if (stencilTiles && !stencilTiles->empty()) {
        visitDrawables([&](const gfx::Drawable& drawable) {
            if (drawable.getEnabled() && drawable.getIs3D() && drawable.hasRenderPass(parameters.pass)) {
                features3d = true;
            }
        });
    }

#if !defined(NDEBUG)
    const auto debugGroupRender = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // If we're doing 3D stenciling and have any features to draw, set up a stencil mask per
    // shader, lazily, the first time each distinct shader is encountered below.
    // If we're doing 2D stenciling and have any drawables with tile IDs, render each tile into
    // the stencil buffer with a different value.
    // We can keep the depth-based descriptors, but the stencil-based ones can change
    // every time, as a new value is assigned in each call to `stencilModeFor3D`.
    std::unordered_map<const void*, gfx::StencilMode> stencilModesByShader;
    std::unordered_map<const void*, MTLDepthStencilStatePtr> stateStencilByShader, stateDepthStencilByShader;
    std::function<const MTLDepthStencilStatePtr&(bool, bool, const void*)> getDepthStencilState;
    if (features3d) {
        const auto stencilModeFor = [&](const void* shaderKey) -> const gfx::StencilMode& {
            auto it = stencilModesByShader.find(shaderKey);
            if (it == stencilModesByShader.end()) {
                it = stencilModesByShader.emplace(shaderKey, parameters.stencilModeFor3D()).first;
            }
            return it->second;
        };
        // If we're using group-wide states, build only the ones that actually get used
        getDepthStencilState =
            [&](bool depth, bool stencil, const void* shaderKey) -> const MTLDepthStencilStatePtr& {
            if (depth) {
                // We assume this doesn't change over the lifetime of a layer group.
                const auto depthMode = parameters.depthModeFor3D();
                if (stencil) {
                    auto it = stateDepthStencilByShader.find(shaderKey);
                    if (it == stateDepthStencilByShader.end()) {
                        it = stateDepthStencilByShader
                                 .emplace(shaderKey,
                                          context.makeDepthStencilState(depthMode, stencilModeFor(shaderKey), renderable))
                                 .first;
                    }
                    return it->second;
                } else {
                    if (!stateDepth) {
                        stateDepth = context.makeDepthStencilState(depthMode, gfx::StencilMode::disabled(), renderable);
                    }
                    return *stateDepth;
                }
            } else {
                if (stencil) {
                    auto it = stateStencilByShader.find(shaderKey);
                    if (it == stateStencilByShader.end()) {
                        it = stateStencilByShader
                                 .emplace(shaderKey,
                                          context.makeDepthStencilState(
                                              gfx::DepthMode::disabled(), stencilModeFor(shaderKey), renderable))
                                 .first;
                    }
                    return it->second;
                } else {
                    if (!stateNone) {
                        stateNone = context.makeDepthStencilState(
                            gfx::DepthMode::disabled(), gfx::StencilMode::disabled(), renderable);
                    }
                    return *stateNone;
                }
            }
        };
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

        // For layer groups with 3D features, enable either this drawable's shader's stencil
        // mode (lazily allocated the first time each distinct shader is seen, so different
        // shaders in the same layer group get independent dedup pools/refs) or disable
        // stenciling. 2D drawables will set their own stencil mode within `draw`.
        if (features3d) {
            const void* shaderKey = drawable.getShader().get();
            const auto& state = getDepthStencilState(drawable.getEnableDepth(), drawable.getEnableStencil(), shaderKey);
            renderPass.setDepthStencilState(state);
            if (drawable.getEnableStencil()) {
                const auto modeIt = stencilModesByShader.find(shaderKey);
                assert(modeIt != stencilModesByShader.end());
                if (modeIt != stencilModesByShader.end()) {
                    renderPass.setStencilReference(modeIt->second.ref);
                }
            }
        }

        drawable.draw(parameters);
    });
}

} // namespace mtl
} // namespace mln
