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
#include <mbgl/util/hash.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>

#include <cmath>

namespace mbgl {

RenderTarget::RenderTarget(gfx::Context& context_,
                           const Size size,
                           const gfx::TextureChannelDataType type,
                           const bool stencil)
    : context(context_) {
    offscreenTexture = context.createOffscreenTexture(size, type, /*depth=*/true, stencil);
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

void RenderTarget::setDrapeTileID(const UnwrappedTileID& id) {
    drapeTileID = id;
    const auto z = static_cast<float>(id.canonical.z);
    drapeTileValues = {z,
                       static_cast<float>(id.canonical.x) + static_cast<float>(id.wrap) * std::exp2(z),
                       static_cast<float>(id.canonical.y),
                       1.0f};
}

void RenderTarget::updateDrapeGlobalUBO(const shaders::GlobalPaintParamsUBO& params, gfx::Context& context_) {
    if (!drapeTileID) {
        return;
    }
    shaders::GlobalPaintParamsUBO drapeParams = params;
    drapeParams.drape_tile = drapeTileValues;
    if (!drapeGlobalUniformBuffer) {
        drapeGlobalUniformBuffer = context_.createUniformBuffer(&drapeParams, sizeof(drapeParams), true);
    } else {
        drapeGlobalUniformBuffer->update(&drapeParams, sizeof(drapeParams));
    }
}

RenderTarget::DrapeCoverage RenderTarget::computeDrapeCoverage(RenderOrchestrator& orchestrator,
                                                               const PaintParameters& parameters) const {
    DrapeCoverage coverage;
    coverage.totalGroups = 0;
    coverage.zoom = parameters.state.getZoom();
    coverage.propertiesEpoch = LayerTweaker::getPropertiesEpoch();
    orchestrator.visitLayerGroups([&](LayerGroupBase& layerGroup) {
        if (layerGroup.getType() != LayerGroupBase::Type::TileLayerGroup || !layerGroup.shouldRenderToTerrain()) {
            return;
        }
        coverage.totalGroups++;
        bool haveExactOrDescendant = false;
        std::optional<UnwrappedTileID> bestAncestor;
        static_cast<TileLayerGroup&>(layerGroup).visitDrawables([&](const gfx::Drawable& drawable) {
            if (!drawable.getEnabled() || !drawable.getTileID()) {
                return;
            }
            const UnwrappedTileID unwrapped = drawable.getTileID()->toUnwrapped();
            if (unwrapped == *drapeTileID || unwrapped.isChildOf(*drapeTileID)) {
                haveExactOrDescendant = true;
            } else if (drapeTileID->isChildOf(unwrapped)) {
                if (!bestAncestor || unwrapped.canonical.z > bestAncestor->canonical.z) {
                    bestAncestor = unwrapped;
                }
            } else {
                return; // no overlap: contributes nothing to this target
            }
            // Identify the content by drawable id, so a tile loading, unloading
            // or being rebuilt all change the signature
            util::hash_combine(coverage.contentHash, drawable.getID().id());
        });
        if (haveExactOrDescendant || bestAncestor) {
            coverage.groupsWithContent++;
            // Only the coarsest standalone fallback counts as lost detail; when an
            // exact or deeper tile is present the ancestor is clipped away by the
            // tile masks and costs nothing.
            if (!haveExactOrDescendant) {
                coverage.zoomDeficit += drapeTileID->canonical.z - bestAncestor->canonical.z;
            }
        }
    });
    return coverage;
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

    // Enable every drawable whose tile overlaps this target and disable the rest,
    // remembering the previous state so unrelated enable flags are not clobbered.
    // Every overlapping tile is drawn, as in gl-js (coordsAscending): a parent
    // standing in for unloaded children renders into each child target it covers,
    // so a target is only empty when the source genuinely has nothing for it.
    // Where a parent overlaps already-loaded children, the layer groups' own tile
    // clipping masks resolve it, exactly as they do in the main passes - drape
    // targets carry a stencil attachment for this, and the masks are built with
    // the drape placement by PaintParameters::clipMatrixForTile.
    std::vector<std::pair<gfx::Drawable*, bool>> savedEnabled;
    visitDrapedGroups(visitForward, [&](LayerGroupBase& layerGroup) {
        static_cast<TileLayerGroup&>(layerGroup).visitDrawables([&](gfx::Drawable& drawable) {
            savedEnabled.emplace_back(&drawable, drawable.getEnabled());
            bool overlaps = false;
            if (const auto& tileID = drawable.getTileID()) {
                const UnwrappedTileID unwrapped = tileID->toUnwrapped();
                overlaps = unwrapped == *drapeTileID || unwrapped.isChildOf(*drapeTileID) ||
                           drapeTileID->isChildOf(unwrapped);
            }
            drawable.setEnabled(drawable.getEnabled() && overlaps);
        });
    });

    const auto layerGroupCount = orchestrator.numLayerGroups();
    parameters.depthRangeSize = 1 -
                                (layerGroupCount + 2) * PaintParameters::numSublayers * PaintParameters::depthEpsilon;

    // Number the two passes over the same (draped-only) group sequence. The opaque pass
    // below assigns 0..drapedCount-1 (reversed visit), so the translucent pass must start
    // at drapedCount-1 (forward visit) for each group to keep the SAME currentLayer in
    // both passes. Starting at the orchestrator's total group count (which includes the
    // non-draped symbol groups) skews depthModeForSublayer's per-layer depth: translucent
    // fills then test at depths beyond what the opaque pass wrote and are discarded — on
    // styles with many symbol layers this silently dropped all low-stack draped fills
    // (landcover, parks, water).
    size_t drapedCount = 0;
    visitDrapedGroups(visitForward, [&](LayerGroupBase&) { drapedCount++; });

    // draw draped layer groups, opaque pass
    parameters.pass = RenderPass::Opaque;
    parameters.currentLayer = 0;
    visitDrapedGroups(visitReversed, [&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        parameters.currentLayer++;
    });

    // draw draped layer groups, translucent pass
    parameters.pass = RenderPass::Translucent;
    parameters.currentLayer = drapedCount > 0 ? static_cast<uint32_t>(drapedCount) - 1 : 0;
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
    if (drapeTileID) {
        const DrapeCoverage coverage = computeDrapeCoverage(orchestrator, parameters);

        // Render cache: a drape is rendered with a tile-local orthographic matrix,
        // so its content does not depend on where the camera is - only on which
        // drawables cover this tile, the zoom (draped UBOs carry zoom-derived
        // values), and the evaluated properties. When none of those changed, the
        // texture is already correct: keep it. This is what makes panning cheap,
        // since panning changes none of them, and it is the maplibre-gl-js
        // behaviour (render a terrain tile's texture only when its stack changes).
        if (coverage.sameContentAs(bakedCoverage)) {
            return;
        }

        // Otherwise the content did change. Keep what is already baked when the new
        // content would be strictly worse (fewer draped layers with content, or
        // coarser ancestor fallbacks): while browsing, a tile's content briefly
        // drops out of the render set (eviction, reload) and re-rendering would
        // flash the drape empty before it recovers. A change in evaluated
        // properties is exempt and always re-renders, because it is authoritative:
        // a style edit that removes a draped layer is legitimately "worse" and must
        // not be held back forever. Otherwise the target's lifetime bounds
        // staleness: when its terrain tile leaves the cover it is destroyed.
        const bool propertiesChanged = coverage.propertiesEpoch != bakedCoverage.propertiesEpoch;
        if (!propertiesChanged && coverage.worseThan(bakedCoverage)) {
            return;
        }
        bakedCoverage = coverage;
    }

    // Drape targets carry a depth and stencil attachment, as maplibre-gl-js's
    // render-to-texture framebuffer does; both are cleared each frame. Targets that
    // are not draped (e.g. the hillshade prepare pass) have depth only.
    parameters.renderPass = parameters.encoder->createRenderPass(
        "render target",
        {.renderable = *offscreenTexture,
         .clearColor = backgroundColor,
         .clearDepth = 1.0f,
         .clearStencil = drapeTileID ? 0 : std::optional<int32_t>{}});
#if MLN_RENDER_BACKEND_OPENGL
    parameters.updateStencilBufferAvailability();
#endif

    if (drapeTileID) {
        // Placement for apply_drape_transform, and for the CPU-side equivalent that
        // builds this target's tile clipping masks (clipMatrixForTile). Set before
        // clearing the stencil, which itself draws a covering quad on some backends.
        parameters.currentDrapeTile = drapeTileValues;
        // Drop clipping masks cached from the main passes or another drape target:
        // the same tile set placed into a different target needs different masks.
        // The render pass above already cleared the stencil buffer itself.
        parameters.invalidateTileClippingMasks();
    }

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
        parameters.currentDrapeTile = {{0, 0, 0, 0}};
        // Leaving drape placement: the masks just built do not apply to what renders next
        parameters.invalidateTileClippingMasks();
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
