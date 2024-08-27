#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gl/renderer_backend.hpp>
#include "android_renderer_backend.hpp"

namespace mbgl {
namespace android {

class AndroidGLRendererBackend : public AndroidRendererBackend,
                                 public gl::RendererBackend,
                                 public mbgl::gfx::Renderable {
public:
    AndroidGLRendererBackend();
    ~AndroidGLRendererBackend() override;

    mbgl::gfx::RendererBackend& getImpl() override { return *this; }

    void updateViewPort() override;

    // Ensures the current context is not cleaned up when destroyed
    void markContextLost() override;

    void resizeFramebuffer(int width, int height) override;
    PremultipliedImage readFramebuffer() override;

    // mbgl::gfx::RendererBackend implementation
public:
    mbgl::gfx::Renderable& getDefaultRenderable() override { return *this; }

protected:
    void activate() override {
        // no-op
    }
    void deactivate() override {
        // no-op
    }

    // mbgl::gl::RendererBackend implementation
protected:
    mbgl::gl::ProcAddress getExtensionFunctionPointer(const char*) override;
    void updateAssumedState() override;
};

} // namespace android
} // namespace mbgl
