#include "glfw_renderer_frontend.hpp"

#include <mln/renderer/renderer.hpp>
#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/util/instrumentation.hpp>

GLFWRendererFrontend::GLFWRendererFrontend(std::unique_ptr<mln::Renderer> renderer_, GLFWView& glfwView_)
    : glfwView(glfwView_),
      renderer(std::move(renderer_)) {
    glfwView.setRenderFrontend(this);
}

GLFWRendererFrontend::~GLFWRendererFrontend() = default;

void GLFWRendererFrontend::reset() {
    assert(renderer);
    renderer.reset();
}

void GLFWRendererFrontend::setObserver(mln::RendererObserver& observer) {
    assert(renderer);
    renderer->setObserver(&observer);
}

void GLFWRendererFrontend::update(std::shared_ptr<mln::UpdateParameters> params) {
    updateParameters = std::move(params);
    glfwView.invalidate();
}

const mln::TaggedScheduler& GLFWRendererFrontend::getThreadPool() const {
    return glfwView.getRendererBackend().getThreadPool();
}

void GLFWRendererFrontend::render() {
    MLN_TRACE_FUNC();

    assert(renderer);

    if (!updateParameters) return;

    mln::gfx::BackendScope guard{glfwView.getRendererBackend(), mln::gfx::BackendScope::ScopeType::Implicit};

    // onStyleImageMissing might be called during a render. The user implemented
    // method could trigger a call to MLNRenderFrontend#update which overwrites
    // `updateParameters`. Copy the shared pointer here so that the parameters
    // aren't destroyed while `render(...)` is still using them.
    auto updateParameters_ = updateParameters;
    renderer->render(updateParameters_);
}

mln::Renderer* GLFWRendererFrontend::getRenderer() {
    assert(renderer);
    return renderer.get();
}
