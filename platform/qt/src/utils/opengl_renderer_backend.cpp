#if defined(MLN_RENDER_BACKEND_OPENGL)

#include "opengl_renderer_backend.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gl/renderable_resource.hpp>

#include <QOpenGLContext>

#include <QtGlobal>

namespace QMapLibre {

class RenderableResource final : public mbgl::gl::RenderableResource {
public:
    explicit RenderableResource(OpenGLRendererBackend& backend_)
        : backend(backend_) {}

    void bind() override {
        assert(mbgl::gfx::BackendScope::exists());
        backend.restoreFramebufferBinding();
        backend.setViewport(0, 0, backend.getSize());
    }

private:
    OpenGLRendererBackend& backend;
};

OpenGLRendererBackend::OpenGLRendererBackend(const mbgl::gfx::ContextMode mode)
    : mbgl::gl::RendererBackend(mode),
      mbgl::gfx::Renderable({0, 0}, std::make_unique<RenderableResource>(*this)) {}

OpenGLRendererBackend::~OpenGLRendererBackend() = default;

void OpenGLRendererBackend::updateAssumedState() {
    assumeFramebufferBinding(ImplicitFramebufferBinding);
    assumeViewport(0, 0, size);
}

void OpenGLRendererBackend::restoreFramebufferBinding() {
    setFramebufferBinding(m_fbo);
}

void OpenGLRendererBackend::updateFramebuffer(uint32_t fbo, const mbgl::Size& newSize) {
    m_fbo = fbo;
    size = newSize;
}

mbgl::gl::ProcAddress OpenGLRendererBackend::getExtensionFunctionPointer(const char* name) {
    QOpenGLContext* thisContext = QOpenGLContext::currentContext();
    return thisContext->getProcAddress(name);
}

void OpenGLRendererBackend::activate() {
    // Qt has already made the context current for us (QOpenGLWidget / QSG).
    // Nothing to do here.
}

void OpenGLRendererBackend::deactivate() {
    // No explicit teardown necessary. If desired we could call makeCurrent on
    // nullptr to release the context, but Qt typically handles that.
}

} // namespace QMapLibre

#endif // MLN_RENDER_BACKEND_OPENGL
