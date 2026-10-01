#include "renderer_frontend.hpp"

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/util/instrumentation.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/string.hpp>

#include <cassert>
#include <exception>
#include <utility>

namespace mln {
namespace ohos {

RendererFrontend::RendererFrontend(gfx::RendererBackend& backend_,
                                   const float pixelRatio,
                                   std::optional<std::string> localFontFamily)
    : backend(backend_),
      renderer(std::make_unique<Renderer>(backend, pixelRatio, std::move(localFontFamily))) {}

RendererFrontend::~RendererFrontend() = default;

void RendererFrontend::reset() {
    renderer.reset();
    updateParameters.reset();
    needsRender = false;
}

void RendererFrontend::setObserver(RendererObserver& observer) {
    assert(renderer);
    renderer->setObserver(&observer);
}

void RendererFrontend::update(std::shared_ptr<UpdateParameters> params) {
    updateParameters = std::move(params);
    needsRender = true;
}

const TaggedScheduler& RendererFrontend::getThreadPool() const {
    return backend.getThreadPool();
}

bool RendererFrontend::renderFrame() {
    MLN_TRACE_FUNC();

    if (!renderer || !updateParameters || !needsRender) {
        return false;
    }

    needsRender = false;
    try {
        gfx::BackendScope guard{backend};
        auto params = updateParameters;
        renderer->render(params);
        return true;
    } catch (...) {
        needsRender = true;
        Log::Error(Event::Render, util::toString(std::current_exception()));
        return false;
    }
}

bool RendererFrontend::hasPendingRender() const {
    return renderer && updateParameters && needsRender;
}

void RendererFrontend::setTileCacheEnabled(bool enable) {
    if (renderer) {
        renderer->setTileCacheEnabled(enable);
    }
}

void RendererFrontend::reduceMemoryUse() {
    if (renderer) {
        renderer->reduceMemoryUse();
    }
}

} // namespace ohos
} // namespace mln
