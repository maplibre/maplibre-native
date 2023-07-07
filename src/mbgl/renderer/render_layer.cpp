#include <mbgl/renderer/render_layer.hpp>

#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/util/logging.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/renderer/layer_group.hpp>
#endif

namespace mbgl {

using namespace style;

RenderLayer::RenderLayer(Immutable<style::LayerProperties> properties)
    : evaluatedProperties(std::move(properties)),
      baseImpl(evaluatedProperties->baseImpl) {}

void RenderLayer::transition(const TransitionParameters& parameters, Immutable<style::Layer::Impl> newImpl) {
    baseImpl = std::move(newImpl);
    transition(parameters);
}

bool RenderLayer::needsPlacement() const {
    return baseImpl->getTypeInfo()->crossTileIndex == style::LayerTypeInfo::CrossTileIndex::Required &&
           !placementData.empty();
}

const std::string& RenderLayer::getID() const {
    return baseImpl->id;
}

int32_t RenderLayer::getLayerIndex() const noexcept {
    return layerIndex;
}

bool RenderLayer::hasRenderPass(RenderPass pass) const {
    return passes & pass;
}

bool RenderLayer::needsRendering() const {
    return passes != RenderPass::None && baseImpl->visibility != style::VisibilityType::None;
}

bool RenderLayer::supportsZoom(float zoom) const {
    // TODO: shall we use rounding or epsilon comparisons?
    return baseImpl->minZoom <= zoom && baseImpl->maxZoom >= zoom;
}

void RenderLayer::prepare(const LayerPrepareParameters& params) {
    assert(params.source);
    assert(params.source->isEnabled());
    renderTiles = params.source->getRenderTiles();
    addRenderPassesFromTiles();

#if MLN_DRAWABLE_RENDERER
    updateRenderTileIDs();
#endif // MLN_DRAWABLE_RENDERER
}

std::optional<Color> RenderLayer::getSolidBackground() const {
    return std::nullopt;
}

#if MLN_DRAWABLE_RENDERER
void RenderLayer::layerChanged(const TransitionParameters&,
                               const Immutable<style::Layer::Impl>&,
                               UniqueChangeRequestVec& changes) {
    // Treat a layer change the same as a remove.
    // It will be set up again when `update()` is called.
    layerRemoved(changes);
}

void RenderLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    removeAllDrawables();
    activateLayerGroup(layerGroup, false, changes);
}
#endif

void RenderLayer::markContextDestroyed() {
    // no-op
}

void RenderLayer::checkRenderability(const PaintParameters& parameters, const uint32_t activeBindingCount) {
    // Only warn once for every layer.
    if (hasRenderFailures) {
        return;
    }

    if (activeBindingCount > parameters.context.maximumVertexBindingCount) {
        Log::Error(Event::OpenGL,
                   "The layer '" + getID() +
                       "' uses more data-driven properties than the current device "
                       "supports, and will have rendering errors. To ensure "
                       "compatibility with this "
                       "device, use " +
                       std::to_string(activeBindingCount - gfx::Context::minimumRequiredVertexBindingCount) +
                       " fewer data driven properties in this layer.");
        hasRenderFailures = true;
    } else if (activeBindingCount > gfx::Context::minimumRequiredVertexBindingCount) {
        Log::Warning(Event::OpenGL,
                     "The layer '" + getID() +
                         "' uses more data-driven properties than some devices may "
                         "support. "
                         "Though it will render correctly on this device, it may have "
                         "rendering errors "
                         "on other devices. To ensure compatibility with all devices, "
                         "use " +
                         std::to_string(activeBindingCount - gfx::Context::minimumRequiredVertexBindingCount) +
                         "fewer "
                         "data-driven properties in this layer.");
        hasRenderFailures = true;
    }
}

void RenderLayer::addRenderPassesFromTiles() {
    assert(renderTiles);
    for (const RenderTile& tile : *renderTiles) {
        if (const LayerRenderData* renderData = tile.getLayerRenderData(*baseImpl)) {
            passes |= RenderPass(renderData->layerProperties->renderPasses);
        }
    }
}

const LayerRenderData* RenderLayer::getRenderDataForPass(const RenderTile& tile, RenderPass pass) const {
    if (const LayerRenderData* renderData = tile.getLayerRenderData(*baseImpl)) {
        return bool(RenderPass(renderData->layerProperties->renderPasses) & pass) ? renderData : nullptr;
    }
    return nullptr;
}

#if MLN_DRAWABLE_RENDERER
void RenderLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    if (const auto tileGroup = static_cast<TileLayerGroup*>(layerGroup.get())) {
        stats.drawablesRemoved += tileGroup->removeDrawables(renderPass, tileID).size();
    }
}

void RenderLayer::removeAllDrawables() {
    if (layerGroup) {
        stats.drawablesRemoved += layerGroup->getDrawableCount();
        layerGroup->clearDrawables();
    }
}

void RenderLayer::updateRenderTileIDs() {
    renderTileIDs.clear();
    if (renderTiles) {
        const auto inserter = std::inserter(renderTileIDs, renderTileIDs.end());
        const auto getID = [](const auto& tile) {
            return tile.get().getOverscaledTileID();
        };
        renderTileIDs.reserve(renderTiles->size());
        std::transform(renderTiles->begin(), renderTiles->end(), inserter, getID);
    }
}

void RenderLayer::layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec& changes) {
    layerIndex = newLayerIndex;

    // Submit a change request to update the layer index of our tile layer group
    if (layerGroup) {
        changes.emplace_back(std::make_unique<UpdateLayerGroupIndexRequest>(layerGroup, newLayerIndex));
    }
}

void RenderLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    isRenderable = willRender;

    // This layer is either being freshly included in the renderable set or excluded
    activateLayerGroup(layerGroup, willRender, changes);
}

void RenderLayer::setLayerGroup(LayerGroupBasePtr layerGroup_, UniqueChangeRequestVec& changes) {
    // Remove the active layer group, if any, before replacing it.
    activateLayerGroup(layerGroup, false, changes);

    layerGroup = std::move(layerGroup_);

    // Add the new layer group, if we're currently renderable.
    activateLayerGroup(layerGroup, isRenderable, changes);
}

/// (Un-)Register the layer group with the orchestrator
void RenderLayer::activateLayerGroup(const LayerGroupBasePtr& layerGroup_,
                                     bool activate,
                                     UniqueChangeRequestVec& changes) {
    if (layerGroup_) {
        if (activate) {
            // The RenderTree has determined this layer should be included in the renderable set for a frame
            changes.emplace_back(std::make_unique<AddLayerGroupRequest>(layerGroup_, /*canReplace=*/true));
        } else {
            // The RenderTree is informing us we should not render anything
            changes.emplace_back(std::make_unique<RemoveLayerGroupRequest>(layerGroup_->getLayerIndex()));
        }
    }
}
#endif

} // namespace mbgl
