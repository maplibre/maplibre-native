#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

namespace mbgl {

AddLayerGroupRequest::AddLayerGroupRequest(LayerGroupBasePtr layerGroup_)
    : layerGroup(std::move(layerGroup_)) {}

AddLayerGroupRequest::AddLayerGroupRequest(AddLayerGroupRequest &&other)
    : layerGroup(std::move(other.layerGroup)) {}

void AddLayerGroupRequest::execute(RenderOrchestrator &orchestrator) {
    mbgl::Log::Info(mbgl::Event::General, "AddLayerGroupRequest::execute - adding layer group: " + 
                    (layerGroup ? layerGroup->getName() : "null"));
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

} // namespace mbgl
