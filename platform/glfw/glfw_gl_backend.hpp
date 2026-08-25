#pragma once

#include "glfw_backend.hpp"

#include <mln/gfx/renderable.hpp>
#include <mln/gl/renderer_backend.hpp>

struct GLFWwindow;

class GLFWGLBackend final : public GLFWBackend, public mln::gl::RendererBackend, public mln::gfx::Renderable {
public:
    GLFWGLBackend(GLFWwindow*, bool capFrameRate);
    ~GLFWGLBackend() override;

    void swap();

    // GLFWRendererBackend implementation
public:
    mln::gfx::RendererBackend& getRendererBackend() override { return *this; }
    mln::Size getSize() const override;
    void setSize(mln::Size) override;

    // mln::gfx::RendererBackend implementation
public:
    mln::gfx::Renderable& getDefaultRenderable() override { return *this; }

protected:
    void activate() override;
    void deactivate() override;

    // mln::gl::RendererBackend implementation
protected:
    mln::gl::ProcAddress getExtensionFunctionPointer(const char*) override;
    void updateAssumedState() override;

private:
    GLFWwindow* window;
};
