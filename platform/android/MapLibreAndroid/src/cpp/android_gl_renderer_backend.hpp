#pragma once

#include <mln/gfx/renderable.hpp>
#include <mln/gl/renderer_backend.hpp>
#include "android_renderer_backend.hpp"

namespace mln {
namespace android {

class AndroidGLRendererBackend : public AndroidRendererBackend,
                                 public gl::RendererBackend,
                                 public mln::gfx::Renderable {
public:
    AndroidGLRendererBackend();
    ~AndroidGLRendererBackend() override;

    mln::gfx::RendererBackend& getImpl() override { return *this; }

    void updateViewPort() override;

    // Ensures the current context is not cleaned up when destroyed
    void markContextLost() override;

    void resizeFramebuffer(int width, int height) override;
    PremultipliedImage readFramebuffer() override;

    // mln::gfx::RendererBackend implementation
public:
    mln::gfx::Renderable& getDefaultRenderable() override { return *this; }

protected:
    void activate() override {
        // no-op
    }
    void deactivate() override {
        // no-op
    }

    // mln::gl::RendererBackend implementation
protected:
    mln::gl::ProcAddress getExtensionFunctionPointer(const char*) override;
    void updateAssumedState() override;
};

} // namespace android
} // namespace mln
