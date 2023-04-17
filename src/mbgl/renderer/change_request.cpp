#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

namespace mbgl {

void AddDrawableRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.addDrawable(std::move(drawable));
}

void RemoveDrawableRequest::execute(RenderOrchestrator &orchestrator) {
    orchestrator.removeDrawable(drawableId);
}

} // namespace mbgl
