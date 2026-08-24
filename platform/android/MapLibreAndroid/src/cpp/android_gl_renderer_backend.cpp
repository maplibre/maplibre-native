#include "android_gl_renderer_backend.hpp"

#include <mln/gfx/backend_scope.hpp>
#include <mln/gl/context.hpp>
#include <mln/gl/renderable_resource.hpp>

#include <EGL/egl.h>

#include <cassert>

namespace mln {
namespace android {

class AndroidGLRenderableResource final : public mln::gl::RenderableResource {
public:
    AndroidGLRenderableResource(AndroidGLRendererBackend& backend_)
        : backend(backend_) {}

    void bind() override {
        assert(gfx::BackendScope::exists());
        backend.setFramebufferBinding(0);
        backend.setViewport(0, 0, backend.getSize());
    }

    void swap() override {
        const auto& swapBehaviour = static_cast<AndroidGLRendererBackend&>(backend).getSwapBehavior();
        if (swapBehaviour == gfx::Renderable::SwapBehaviour::Flush) {
            static_cast<gl::Context&>(backend.getContext()).finish();
        }
    }

private:
    AndroidGLRendererBackend& backend;
};

AndroidGLRendererBackend::AndroidGLRendererBackend()
    : gl::RendererBackend(gfx::ContextMode::Unique),
      mln::gfx::Renderable({64, 64}, std::make_unique<AndroidGLRenderableResource>(*this)) {}

AndroidGLRendererBackend::~AndroidGLRendererBackend() = default;

gl::ProcAddress AndroidGLRendererBackend::getExtensionFunctionPointer(const char* name) {
    assert(gfx::BackendScope::exists());
    return eglGetProcAddress(name);
}

void AndroidGLRendererBackend::updateViewPort() {
    assert(gfx::BackendScope::exists());
    setViewport(0, 0, size);
}

void AndroidGLRendererBackend::resizeFramebuffer(int width, int height) {
    size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

PremultipliedImage AndroidGLRendererBackend::readFramebuffer() {
    assert(gfx::BackendScope::exists());
    return gl::RendererBackend::readFramebuffer(size);
}

void AndroidGLRendererBackend::updateAssumedState() {
    assumeFramebufferBinding(0);
    assumeViewport(0, 0, size);
}

void AndroidGLRendererBackend::markContextLost() {
    if (context) {
        getContext<gl::Context>().setCleanupOnDestruction(false);
    }
}

} // namespace android
} // namespace mln

namespace mln {
namespace gfx {

template <>
std::unique_ptr<android::AndroidRendererBackend> Backend::Create<mln::gfx::Backend::Type::OpenGL>(ANativeWindow*) {
    return std::make_unique<android::AndroidGLRendererBackend>();
}

} // namespace gfx
} // namespace mln
