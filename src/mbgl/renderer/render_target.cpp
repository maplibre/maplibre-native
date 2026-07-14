#include <mbgl/renderer/render_target.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/offscreen_texture.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/layer_ubo.hpp>

#include <cmath>

namespace mbgl {

RenderTarget::RenderTarget(gfx::Context& context_, const Size size, const gfx::TextureChannelDataType type)
    : context(context_) {
    offscreenTexture = context.createOffscreenTexture(size, type);
    backgroundColor = Color{0.0f, 0.0f, 0.0f, 1.0f};
}

RenderTarget::~RenderTarget() {}

const gfx::Texture2DPtr& RenderTarget::getTexture() {
    return offscreenTexture->getTexture();
};

bool RenderTarget::addLayerGroup(LayerGroupBasePtr layerGroup, const bool replace) {
    const auto index = layerGroup->getLayerIndex();
    const auto result = layerGroupsByLayerIndex.insert(std::make_pair(index, LayerGroupBasePtr{}));
    if (result.second) {
        // added
        result.first->second = std::move(layerGroup);
        return true;
    } else {
        // not added
        if (replace) {
            result.first->second = std::move(layerGroup);
            return true;
        } else {
            return false;
        }
    }
}

bool RenderTarget::removeLayerGroup(const int32_t layerIndex) {
    const auto hit = layerGroupsByLayerIndex.find(layerIndex);
    if (hit != layerGroupsByLayerIndex.end()) {
        layerGroupsByLayerIndex.erase(hit);
        return true;
    } else {
        return false;
    }
}

size_t RenderTarget::numLayerGroups() const noexcept {
    return layerGroupsByLayerIndex.size();
}

static const LayerGroupBasePtr no_group;

const LayerGroupBasePtr& RenderTarget::getLayerGroup(const int32_t layerIndex) const {
    const auto hit = layerGroupsByLayerIndex.find(layerIndex);
    return (hit == layerGroupsByLayerIndex.end()) ? no_group : hit->second;
}

void RenderTarget::upload(gfx::UploadPass& uploadPass) {
    visitLayerGroups(([&](LayerGroupBase& layerGroup) { layerGroup.upload(uploadPass); }));
}

void RenderTarget::updateDrapeGlobalUBO(const shaders::GlobalPaintParamsUBO& params, gfx::Context& context_) {
    if (!drapeTileID) {
        return;
    }
    shaders::GlobalPaintParamsUBO drapeParams = params;
    const auto z = static_cast<float>(drapeTileID->canonical.z);
    drapeParams.drape_tile = {
        z,
        static_cast<float>(drapeTileID->canonical.x) + static_cast<float>(drapeTileID->wrap) * std::exp2(z),
        static_cast<float>(drapeTileID->canonical.y),
        1.0f};
    if (!drapeGlobalUniformBuffer) {
        drapeGlobalUniformBuffer = context_.createUniformBuffer(&drapeParams, sizeof(drapeParams), true);
    } else {
        drapeGlobalUniformBuffer->update(&drapeParams, sizeof(drapeParams));
    }
}

void RenderTarget::renderDrapedLayerGroups(RenderOrchestrator& orchestrator, PaintParameters& parameters) {
    // Following gl-js render_to_texture: every draped layer tile that overlaps this
    // target's tile is drawn into it; the vertex shader places overlapping tiles via
    // the drape_tile fields (see apply_drape_transform), so a parent tile standing in
    // for unloaded children renders into each child target it covers.
    const auto visitDrapedGroups = [&](auto&& visit, auto&& f) {
        visit([&](LayerGroupBase& layerGroup) {
            if (layerGroup.getType() == LayerGroupBase::Type::TileLayerGroup && layerGroup.shouldRenderToTerrain()) {
                f(layerGroup);
            }
        });
    };
    const auto visitForward = [&](auto&& f) {
        orchestrator.visitLayerGroups(f);
    };
    const auto visitReversed = [&](auto&& f) {
        orchestrator.visitLayerGroupsReversed(f);
    };

    // Enable, per layer group, a single consistent coverage of this target,
    // remembering the previous state so unrelated enable flags are not
    // clobbered. The render tile set can contain overlapping tiles (a parent
    // standing in for missing children next to already-loaded children); the
    // main passes resolve that overlap with stencil clipping, but the drape
    // targets have no stencil attachment, so a blurry parent would draw over
    // its sharp children. Prefer exact matches, then the deepest covering
    // ancestor, then descendants (which cannot overlap each other's area
    // without an ancestor between them being preferred instead).
    std::vector<std::pair<gfx::Drawable*, bool>> savedEnabled;
    visitDrapedGroups(visitForward, [&](LayerGroupBase& layerGroup) {
        auto& tileLayerGroup = static_cast<TileLayerGroup&>(layerGroup);

        bool haveExact = false;
        std::optional<UnwrappedTileID> bestAncestor;
        tileLayerGroup.visitDrawables([&](const gfx::Drawable& drawable) {
            if (!drawable.getEnabled() || !drawable.getTileID()) {
                return;
            }
            const UnwrappedTileID unwrapped = drawable.getTileID()->toUnwrapped();
            if (unwrapped == *drapeTileID) {
                haveExact = true;
            } else if (drapeTileID->isChildOf(unwrapped)) {
                if (!bestAncestor || unwrapped.canonical.z > bestAncestor->canonical.z) {
                    bestAncestor = unwrapped;
                }
            }
        });

        tileLayerGroup.visitDrawables([&](gfx::Drawable& drawable) {
            savedEnabled.emplace_back(&drawable, drawable.getEnabled());
            bool enable = false;
            if (const auto& tileID = drawable.getTileID()) {
                const UnwrappedTileID unwrapped = tileID->toUnwrapped();
                if (haveExact) {
                    enable = unwrapped == *drapeTileID;
                } else if (bestAncestor) {
                    enable = unwrapped == *bestAncestor;
                } else {
                    enable = unwrapped.isChildOf(*drapeTileID);
                }
            }
            drawable.setEnabled(drawable.getEnabled() && enable);
        });
    });

    const auto layerGroupCount = orchestrator.numLayerGroups();
    parameters.depthRangeSize = 1 -
                                (layerGroupCount + 2) * PaintParameters::numSublayers * PaintParameters::depthEpsilon;

    // draw draped layer groups, opaque pass
    parameters.pass = RenderPass::Opaque;
    parameters.currentLayer = 0;
    visitDrapedGroups(visitReversed, [&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        parameters.currentLayer++;
    });

    // draw draped layer groups, translucent pass
    parameters.pass = RenderPass::Translucent;
    parameters.currentLayer = static_cast<uint32_t>(layerGroupCount) - 1;
    visitDrapedGroups(visitForward, [&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        if (parameters.currentLayer > 0) {
            parameters.currentLayer--;
        }
    });

    for (const auto& [drawable, enabled] : savedEnabled) {
        drawable->setEnabled(enabled);
    }
}

void RenderTarget::render(RenderOrchestrator& orchestrator, const RenderTree& renderTree, PaintParameters& parameters) {
    // The offscreen target has a depth attachment (no stencil), matching maplibre-gl-js's
    // drape framebuffer, so clear depth each frame. Stencil is not present.
    parameters.renderPass = parameters.encoder->createRenderPass(
        "render target",
        {.renderable = *offscreenTexture, .clearColor = backgroundColor, .clearDepth = 1.0f, .clearStencil = {}});
#if MLN_RENDER_BACKEND_OPENGL
    parameters.updateStencilBufferAvailability();
#endif

    // For drape targets, swap in this target's copy of the global paint params,
    // which carries the target tile in `drape_tile` for apply_drape_transform.
    auto& globalUniforms = context.mutableGlobalUniformBuffers();
    gfx::UniformBufferPtr previousGlobalPaintParams;
    if (drapeTileID && drapeGlobalUniformBuffer) {
        previousGlobalPaintParams = globalUniforms.get(shaders::idGlobalPaintParamsUBO);
        globalUniforms.set(shaders::idGlobalPaintParamsUBO, drapeGlobalUniformBuffer);
    }

    // The main render passes bind these in Renderer::Impl::render, but this pass runs
    // before that; without the bind, drawables needing e.g. GlobalPaintParamsUBO are
    // rejected by validating drivers ("DrawElements: ValidateState() failed").
    context.bindGlobalUniformBuffers(*parameters.renderPass);

    const gfx::ScissorRect prevScissorRect = parameters.scissorRect;
    const auto& size = getTexture()->getSize();
    parameters.scissorRect = {.x = 0, .y = 0, .width = size.width, .height = size.height};

    if (drapeTileID) {
        // Terrain drape target: render the orchestrator's draped layer groups
        // (their tweakers already ran in the main layer group update)
        renderDrapedLayerGroups(orchestrator, parameters);
    } else {
        // Run layer tweakers to update any dynamic elements
        parameters.currentLayer = 0;
        visitLayerGroups([&](LayerGroupBase& layerGroup) {
            layerGroup.runTweakers(renderTree, parameters);
            parameters.currentLayer++;
        });

        // draw layer groups, opaque pass
        parameters.pass = RenderPass::Opaque;
        parameters.depthRangeSize = 1 - (numLayerGroups() + 2) * PaintParameters::numSublayers *
                                            PaintParameters::depthEpsilon;

        parameters.currentLayer = 0;
        visitLayerGroupsReversed([&](LayerGroupBase& layerGroup) {
            layerGroup.render(orchestrator, parameters);
            parameters.currentLayer++;
        });

        // draw layer groups, translucent pass
        parameters.pass = RenderPass::Translucent;
        parameters.depthRangeSize = 1 - (numLayerGroups() + 2) * PaintParameters::numSublayers *
                                            PaintParameters::depthEpsilon;

        parameters.currentLayer = static_cast<uint32_t>(numLayerGroups()) - 1;
        visitLayerGroups([&](LayerGroupBase& layerGroup) {
            layerGroup.render(orchestrator, parameters);
            if (parameters.currentLayer > 0) {
                parameters.currentLayer--;
            }
        });
    }

    context.unbindGlobalUniformBuffers(*parameters.renderPass);
    if (previousGlobalPaintParams) {
        globalUniforms.set(shaders::idGlobalPaintParamsUBO, std::move(previousGlobalPaintParams));
    }

    parameters.renderPass.reset();
    parameters.encoder->present(*offscreenTexture);

    parameters.scissorRect = prevScissorRect;
}

} // namespace mbgl
