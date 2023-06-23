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

UpdateLayerGroupIndexRequest::UpdateLayerGroupIndexRequest(LayerGroupBasePtr tileLayerGroup_, int32_t newLayerIndex_)
    : layerGroup(std::move(tileLayerGroup_)),
      newLayerIndex(newLayerIndex_) {}

void UpdateLayerGroupIndexRequest::execute(RenderOrchestrator &orchestrator) {
    // Update the index of a tile layer group and indicate to the
    // orchestrator that it must rebuild the map of ordered layer groups
    layerGroup->updateLayerIndex(newLayerIndex);
    orchestrator.markLayerGroupOrderDirty();
}

} // namespace mbgl
