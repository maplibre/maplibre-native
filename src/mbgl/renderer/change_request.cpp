#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

namespace mbgl {

AddLayerGroupRequest::AddLayerGroupRequest(LayerGroupPtr layerGroup_, bool canReplace)
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

} // namespace mbgl
