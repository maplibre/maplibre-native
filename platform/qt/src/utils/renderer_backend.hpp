#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gl/renderer_backend.hpp>

#include <QtGlobal>

namespace QMapLibre {

class RendererBackend final : public mbgl::gl::RendererBackend, public mbgl::gfx::Renderable {
public:
    explicit RendererBackend(mbgl::gfx::ContextMode mode);
    ~RendererBackend() override;

    void updateFramebuffer(quint32 fbo, const mbgl::Size &newSize);
    void restoreFramebufferBinding();

    // mbgl::gfx::RendererBackend implementation
public:
    mbgl::gfx::Renderable &getDefaultRenderable() override { return *this; }

protected:
    // No-op, implicit mode.
    void activate() override {}
    void deactivate() override {}

    // mbgl::gl::RendererBackend implementation
protected:
    mbgl::gl::ProcAddress getExtensionFunctionPointer(const char *name) override;
    void updateAssumedState() override;

private:
    quint32 m_fbo{};

    Q_DISABLE_COPY(RendererBackend)
};

} // namespace QMapLibre
