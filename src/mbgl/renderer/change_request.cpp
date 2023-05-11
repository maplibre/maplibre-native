#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

namespace mbgl {

void AddDrawableRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.addDrawable(std::move(drawable));
}

void RemoveDrawableRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.removeDrawable(id);
}

void ResetColorRequest::execute(RenderOrchestrator &orchestrator) {
    if (auto &drawable = orchestrator.getDrawable(id)) {
        drawable->resetColor(newColor);
    }
}

AddLayerGroupRequest::AddLayerGroupRequest(UniqueLayerGroup &&layerGroup_, bool canReplace)
    : layerGroup(std::move(layerGroup_)),
      replace(canReplace) {}

AddLayerGroupRequest::AddLayerGroupRequest(AddLayerGroupRequest &&other)
    : layerGroup(std::move(other.layerGroup)) {}

void AddLayerGroupRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.addLayerGroup(std::move(layerGroup), replace);
}

void RemoveLayerGroupRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.removeLayerGroup(id);
}

} // namespace mbgl
