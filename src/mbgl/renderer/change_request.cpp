#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

namespace mbgl {

void AddDrawableRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.addDrawable(std::move(drawable));
}

void RemoveDrawableRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.removeDrawable(drawableID);
}

void ResetColorRequest::execute(RenderOrchestrator &orchestrator) {
    if (auto &drawable = orchestrator.getDrawable(drawableID)) {
        drawable->resetColor(newColor);
    }
}

} // namespace mbgl
