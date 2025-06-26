#pragma once

#if defined(MLN_RENDER_BACKEND_OPENGL)

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gl/renderer_backend.hpp>
#include <mbgl/util/size.hpp>

namespace QMapLibre {

/*!
    \brief Qt specific OpenGL backend implementation.

    This class provides the glue between Qt's OpenGL context and
    MapLibre Native's generic OpenGL renderer backend.

*/
class OpenGLRendererBackend : public mbgl::gl::RendererBackend, public mbgl::gfx::Renderable {
public:
    explicit OpenGLRendererBackend(const mbgl::gfx::ContextMode mode = mbgl::gfx::ContextMode::Unique);
    ~OpenGLRendererBackend() override;

    // mbgl::gfx::RendererBackend -------------------------------------------------
    mbgl::gfx::Renderable& getDefaultRenderable() override { return *this; }

    // mbgl::gl::RendererBackend --------------------------------------------------
    void updateAssumedState() override;

protected:
    mbgl::gl::ProcAddress getExtensionFunctionPointer(const char*) override;

public:
    // Qt integration helpers -----------------------------------------------------
    void restoreFramebufferBinding();
    void updateFramebuffer(uint32_t fbo, const mbgl::Size& newSize);

private:
    uint32_t m_fbo{0};
};

} // namespace QMapLibre

#endif // MLN_RENDER_BACKEND_OPENGL 