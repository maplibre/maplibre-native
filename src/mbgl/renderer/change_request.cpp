#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

namespace mbgl {

AddLayerGroupRequest::AddLayerGroupRequest(LayerGroupBasePtr layerGroup_)
    : layerGroup(std::move(layerGroup_)) {}

AddLayerGroupRequest::AddLayerGroupRequest(AddLayerGroupRequest &&other)
    : layerGroup(std::move(other.layerGroup)) {}

void AddLayerGroupRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.addLayerGroup(std::move(layerGroup));
}

void RemoveLayerGroupRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.removeLayerGroup(layerGroup);
}

UpdateLayerGroupIndexRequest::UpdateLayerGroupIndexRequest(LayerGroupBasePtr tileLayerGroup_, int32_t newLayerIndex_)
    : layerGroup(std::move(tileLayerGroup_)),
      newLayerIndex(newLayerIndex_) {}

void UpdateLayerGroupIndexRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.updateLayerIndex(layerGroup, newLayerIndex);
}

AddRenderTargetRequest::AddRenderTargetRequest(RenderTargetPtr renderTarget_)
    : renderTarget(std::move(renderTarget_)) {}

void AddRenderTargetRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.addRenderTarget(std::move(renderTarget));
}

RemoveRenderTargetRequest::RemoveRenderTargetRequest(RenderTargetPtr renderTarget_)
    : renderTarget(std::move(renderTarget_)) {}

void RemoveRenderTargetRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.removeRenderTarget(renderTarget);
}

#if MLN_DRAWABLE_RENDERER

void activateRenderTarget(const RenderTargetPtr &renderTarget_, bool activate, UniqueChangeRequestVec &changes) {
    if (renderTarget_) {
        if (activate) {
            // The RenderTree has determined this render target should be included in the renderable set for a frame
            changes.emplace_back(std::make_unique<AddRenderTargetRequest>(renderTarget_));
        } else {
            // The RenderTree is informing us we should not render anything
            changes.emplace_back(std::make_unique<RemoveRenderTargetRequest>(renderTarget_));
        }
    }
}

#endif

} // namespace mbgl
