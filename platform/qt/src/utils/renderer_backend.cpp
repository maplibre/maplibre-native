#include "renderer_backend.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gl/renderable_resource.hpp>

#include <QtGlobal>
#include <QOpenGLContext>

class QMapLibreRenderableResource final : public mbgl::gl::RenderableResource {
public:
    QMapLibreRenderableResource(QMapLibreRendererBackend& backend_) : backend(backend_) {
    }

    void bind() override {
        assert(mbgl::gfx::BackendScope::exists());
        backend.restoreFramebufferBinding();
        backend.setViewport(0, 0, backend.getSize());
    }

private:
    QMapLibreRendererBackend& backend;
};

QMapLibreRendererBackend::QMapLibreRendererBackend(const mbgl::gfx::ContextMode contextMode_)
    : mbgl::gl::RendererBackend(contextMode_),
      mbgl::gfx::Renderable({ 0, 0 }, std::make_unique<QMapLibreRenderableResource>(*this)) {
}

QMapLibreRendererBackend::~QMapLibreRendererBackend() = default;

void QMapLibreRendererBackend::updateAssumedState() {
    assumeFramebufferBinding(ImplicitFramebufferBinding);
    assumeViewport(0, 0, size);
}

void QMapLibreRendererBackend::restoreFramebufferBinding() {
    setFramebufferBinding(m_fbo);
}

void QMapLibreRendererBackend::updateFramebuffer(quint32 fbo, const mbgl::Size& newSize) {
    m_fbo = fbo;
    size = newSize;
}

/*!
    Initializes an OpenGL extension function such as Vertex Array Objects (VAOs),
    required by MapLibre GL Native engine.
*/
mbgl::gl::ProcAddress QMapLibreRendererBackend::getExtensionFunctionPointer(const char* name) {
    QOpenGLContext* thisContext = QOpenGLContext::currentContext();
    return thisContext->getProcAddress(name);
}
