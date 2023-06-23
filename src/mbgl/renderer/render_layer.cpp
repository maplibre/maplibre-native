#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/style/layer.hpp>
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
}

std::optional<Color> RenderLayer::getSolidBackground() const {
    return std::nullopt;
}

#if MLN_DRAWABLE_RENDERER
void RenderLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    // Remove everything
    if (tileLayerGroup) {
        changes.emplace_back(std::make_unique<RemoveLayerGroupRequest>(tileLayerGroup->getLayerIndex()));
        tileLayerGroup.reset();
    }
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
    stats.tileDrawablesRemoved += tileLayerGroup->removeDrawables(renderPass, tileID).size();
}

void RenderLayer::removeAllTiles() {
    if (tileLayerGroup) {
        stats.tileDrawablesRemoved += tileLayerGroup->getDrawableCount();
        tileLayerGroup->clearDrawables();
    }
}

void RenderLayer::layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec& changes) {
    layerIndex = newLayerIndex;

    // Submit a change request to update the layer index of our tile layer group
    if (tileLayerGroup) {
        changes.emplace_back(std::make_unique<UpdateLayerGroupIndexRequest>(tileLayerGroup, newLayerIndex));
    }
}

void RenderLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    if (!tileLayerGroup) {
        return;
    }
    if (isRenderable == willRender) {
        return;
    }

    // This layer is either being freshly included in the renderable set or excluded
    isRenderable = willRender;
    if (willRender) {
        // The RenderTree has determined this layer should be included in the renderable set for a frame
        changes.emplace_back(std::make_unique<AddLayerGroupRequest>(tileLayerGroup, /*canReplace=*/true));
    } else {
        // The RenderTree is informing us we should not render anything
        changes.emplace_back(std::make_unique<RemoveLayerGroupRequest>(tileLayerGroup->getLayerIndex()));
    }
}
#endif

} // namespace mbgl
