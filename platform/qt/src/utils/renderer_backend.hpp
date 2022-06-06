#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gl/renderer_backend.hpp>

#include <QtGlobal>

class QMapLibreRendererBackend final : public mbgl::gl::RendererBackend,
                                       public mbgl::gfx::Renderable {
public:
    QMapLibreRendererBackend(mbgl::gfx::ContextMode);
    ~QMapLibreRendererBackend() override;

    void updateFramebuffer(quint32 fbo, const mbgl::Size&);
    void restoreFramebufferBinding();

    // mbgl::gfx::RendererBackend implementation
public:
    mbgl::gfx::Renderable& getDefaultRenderable() override {
        return *this;
    }

protected:
    // No-op, implicit mode.
    void activate() override {}
    void deactivate() override {}

    // mbgl::gl::RendererBackend implementation
protected:
    mbgl::gl::ProcAddress getExtensionFunctionPointer(const char*) override;
    void updateAssumedState() override;

private:
    quint32 m_fbo{};

    Q_DISABLE_COPY(QMapLibreRendererBackend)
};
