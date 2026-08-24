#include <mln/renderer/render_target.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/gfx/offscreen_texture.hpp>
#include <mln/gfx/render_pass.hpp>
#include <mln/gfx/uniform_buffer.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/layer_tweaker.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_orchestrator.hpp>
#include <mln/renderer/render_tree.hpp>
#include <mln/shaders/layer_ubo.hpp>
#include <mln/util/hash.hpp>
#include <mln/util/string.hpp>

#include <cmath>

namespace mln {

// TEMP Stage-2 diagnostic: per-frame count of drape targets (re-)rendered vs skipped (cache
// hit). If `rendered` stays high while panning, the drape cache is not holding - the suspected
// ~4x overdraw source. Fetched and reset once per frame by the renderer.
namespace {
uint32_t gDrapeRendered = 0;
uint32_t gDrapeSkipped = 0;
} // namespace
void fetchDrapeStats(uint32_t& rendered, uint32_t& skipped) {
    rendered = gDrapeRendered;
    skipped = gDrapeSkipped;
    gDrapeRendered = 0;
    gDrapeSkipped = 0;
}

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
    // Integer tile-zoom only, NOT the continuous zoom: a drape tile's rasterized content
    // is stable within a zoom level, so a pinch within one level must not invalidate it
    // every frame. Crossing an integer zoom changes the covering tiles (contentHash) and
    // re-renders anyway. Matches maplibre-gl-js (a terrain tile's texture is per tile-z).
    coverage.zoom = static_cast<double>(static_cast<int32_t>(parameters.state.getZoom()));
    // NOTE: the paint-property epoch is deliberately NOT tracked here. gl-js keys its
    // terrain RTT cache on tile coverage + source revision, never on paint changes.
    // Tracking the global epoch re-rendered every drape target on any paint transition -
    // a single label fading in bumps the epoch every frame - which defeated the cache
    // entirely (measured: all visible drape targets re-rendered every frame while
    // panning). coverage.propertiesEpoch is left at its default and no longer consulted.
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
            const bool overlaps = unwrapped == *drapeTileID || unwrapped.isChildOf(*drapeTileID) ||
                                  drapeTileID->isChildOf(unwrapped);
            if (overlaps) {
                // Key on the COVERING TILE id, NOT the drawable-instance id. A tile's
                // drawables are rebuilt with fresh ids on every bucket update, fade
                // transition and reload, so a drawable-id hash churned every frame during
                // interaction and re-rendered the drape needlessly (the logcat DRAPE
                // diagnostic showed contentHash changing every frame while the covering
                // tiles and zoom were unchanged). The drape only needs re-rendering when
                // the SET of tiles covering it changes - a tile loading/unloading or an
                // integer zoom crossing. This matches maplibre-gl-js, which keys its RTT
                // cache on tile coverage. Summed (order-independent) so visitDrawables
                // iteration order does not matter.
                std::size_t tileHash = 0;
                util::hash_combine(tileHash, unwrapped.wrap);
                util::hash_combine(tileHash, unwrapped.canonical.z);
                util::hash_combine(tileHash, unwrapped.canonical.x);
                util::hash_combine(tileHash, unwrapped.canonical.y);
                coverage.contentHash += tileHash;
            }
            if (unwrapped == *drapeTileID || unwrapped.isChildOf(*drapeTileID)) {
                haveExactOrDescendant = true;
            } else if (drapeTileID->isChildOf(unwrapped)) {
                if (!bestAncestor || unwrapped.canonical.z > bestAncestor->canonical.z) {
                    bestAncestor = unwrapped;
                }
            } else {
                return; // no overlap: contributes nothing to this target
            }
            // Identify content by the COVERING TILE id, NOT the drawable-instance id: a
            // tile's drawables are rebuilt with fresh ids on every bucket update and fade
            // transition, so a drawable-id hash churned every frame during interaction and
            // re-rendered the drape needlessly (measured: drapes re-rendered every frame while
            // panning -> ~4x overdraw). The drape only needs re-rendering when the SET of
            // covering tiles changes (load/unload/integer-zoom). Summed so visitDrawables
            // order does not matter. Matches maplibre-gl-js (RTT cache keyed on tile coverage).
            std::size_t tileHash = 0;
            util::hash_combine(tileHash, unwrapped.wrap);
            util::hash_combine(tileHash, unwrapped.canonical.z);
            util::hash_combine(tileHash, unwrapped.canonical.x);
            util::hash_combine(tileHash, unwrapped.canonical.y);
            coverage.contentHash += tileHash;
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

RenderTarget::RenderResult RenderTarget::render(RenderOrchestrator& orchestrator,
                                                const RenderTree& renderTree,
                                                PaintParameters& parameters,
                                                bool canRerender) {
    // Render-once targets are hillshade prepare targets (opt-in via setRenderOnce): the DEM
    // texture is baked into the prepare drawable once (RenderHillshadeLayer::update calls
    // setImage), so the DEM->hillshade output is immutable. Render once and keep the
    // offscreen texture - re-running the prepare shader for every tile every frame was the
    // dominant terrain-mode cost (~50ms measured). A new DEM tile produces a fresh target
    // that renders once. This must NOT apply to the terrain depth target (a non-drape
    // target that re-renders on camera movement), hence the explicit opt-in flag rather
    // than keying on !drapeTileID. Matches gl-js prepare-to-FBO reuse.
    if (renderOnce && renderedOnce) {
        return RenderResult::Skipped;
    }
    if (drapeTileID) {
        // Fast path: the per-target coverage scan below is O(draped drawables) and
        // runs for every drape target, so on a busy terrain scene it dominates the
        // frame even when nothing re-renders. But a target's baked content depends
        // only on the drawables overlapping it (plus zoom and the property epoch),
        // captured for every target in one pass as perTargetDrapeSignature. When
        // this target's entry matches what it last evaluated against, no scan can
        // find a difference - skip it. Unlike a single global signature, an
        // unrelated tile loading elsewhere leaves this target's signature untouched,
        // so during movement only the targets that actually changed re-scan.
        std::size_t targetSignature = parameters.drapedContentSignature;
        if (parameters.perTargetDrapeSignature) {
            const auto it = parameters.perTargetDrapeSignature->find(*drapeTileID);
            if (it != parameters.perTargetDrapeSignature->end()) {
                targetSignature = it->second;
            }
        }
        if (bakedSignature && *bakedSignature == targetSignature) {
            return RenderResult::Skipped;
        }

        // Fast path missed: we pay the full O(draped drawables) scan. Count it so
        // the overlay can show whether the short-circuit is actually engaging.
        context.renderingStats().numDrapeCoverageScans++;
        const DrapeCoverage coverage = computeDrapeCoverage(orchestrator, parameters);

        // Render cache: a drape is rendered with a tile-local orthographic matrix,
        // so its content does not depend on where the camera is - only on which
        // drawables cover this tile, the zoom (draped UBOs carry zoom-derived
        // values), and the evaluated properties. When none of those changed, the
        // texture is already correct: keep it. This is what makes panning cheap,
        // since panning changes none of them, and it is the maplibre-gl-js
        // behaviour (render a terrain tile's texture only when its stack changes).
        if (coverage.sameContentAs(bakedCoverage)) {
            bakedSignature = targetSignature;
            return RenderResult::Skipped;
        }

        // Otherwise the content did change. Keep what is already baked when the new
        // content would be strictly worse (fewer draped layers with content, or
        // coarser ancestor fallbacks): while browsing, a tile's content briefly
        // drops out of the render set (eviction, reload) and re-rendering would
        // flash the drape empty before it recovers. A genuine change - a tile's
        // drawable set changing (contentHash) or crossing an integer zoom - is not
        // "worse" and falls through to re-render. The target's lifetime bounds
        // staleness: when its terrain tile leaves the cover it is destroyed.
        if (coverage.worseThan(bakedCoverage)) {
            // Keeping the already-baked (better) content: record that at this
            // signature the decision was to hold, so future identical frames skip
            // the scan too. A real change (drawable set, zoom, properties) moves the
            // signature and re-opens evaluation, preserving the anti-flicker intent.
            bakedSignature = targetSignature;
            return RenderResult::Skipped;
        }

        // Drape render budget: this target needs a re-render, but if the per-frame cap is
        // exhausted (canRerender == false) and it already has a baked texture, defer to a
        // later frame - keep showing the slightly stale texture instead of stalling the
        // frame. Leave bakedCoverage/bakedSignature unchanged so it is re-evaluated and
        // rendered on a subsequent frame. A never-rendered target falls through (rendering
        // it now avoids a blank tile), so bursts of *new* targets are not deferred.
        if (!canRerender && hasRenderedContent) {
            return RenderResult::Deferred;
        }

        bakedCoverage = coverage;
        bakedSignature = targetSignature;

        // Committed to re-rendering this drape (a cache miss). Count it so the
        // measure-first overlay can show drape churn per frame.
        context.renderingStats().numDrapeTargetsRendered++;
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

    // Render-once (hillshade prepare) target baked; skip it on subsequent frames.
    if (renderOnce) {
        renderedOnce = true;
    }

    // This target now holds valid content, so it may be deferred by the drape budget later.
    hasRenderedContent = true;

    parameters.scissorRect = prevScissorRect;
    return RenderResult::Rendered;
}

} // namespace mln
