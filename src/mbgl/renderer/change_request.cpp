#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

namespace mbgl {

AddLayerGroupRequest::AddLayerGroupRequest(LayerGroupBasePtr layerGroup_, bool canReplace)
    : layerGroup(std::move(layerGroup_)),
      replace(canReplace) {}

AddLayerGroupRequest::AddLayerGroupRequest(AddLayerGroupRequest &&other)
    : layerGroup(std::move(other.layerGroup)) {}

void AddLayerGroupRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.addLayerGroup(std::move(layerGroup), replace);
}

void RemoveLayerGroupRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.removeLayerGroup(layerIndex);
}

UpdateLayerGroupIndexRequest::UpdateLayerGroupIndexRequest(std::shared_ptr<TileLayerGroup> tileLayerGroup_,
                                                           int32_t newLayerIndex_)
    : tileLayerGroup(std::move(tileLayerGroup_)),
      newLayerIndex(newLayerIndex_) {}

void UpdateLayerGroupIndexRequest::execute(RenderOrchestrator &orchestrator) {
    // Update the index of a tile layer group and indicate to the orchestator that it must rebuild the map of ordered
    // layer groups
    tileLayerGroup->updateLayerIndex(newLayerIndex);
    orchestrator.markLayerGroupOrderDirty();
}

AddRenderTargetRequest::AddRenderTargetRequest(RenderTargetPtr renderTarget_)
    : renderTarget(std::move(renderTarget_)) {}

void AddRenderTargetRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.addRenderTarget(std::move(renderTarget));
}

RemoveRenderTargetRequest::RemoveRenderTargetRequest(const RenderTargetPtr& renderTarget_)
    : renderTarget(renderTarget_) {}

void RemoveRenderTargetRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.removeRenderTarget(renderTarget);
}

} // namespace mbgl
