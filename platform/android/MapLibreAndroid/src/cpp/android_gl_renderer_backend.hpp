#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gl/renderer_backend.hpp>
#include "android_renderer_backend.hpp"

#include <EGL/egl.h>

namespace mbgl {
namespace android {

class AndroidGLRendererBackend : public AndroidRendererBackend,
                                 public gl::RendererBackend,
                                 public mbgl::gfx::Renderable {
public:
    AndroidGLRendererBackend(bool multiThreadedGpuResourceUpload);
    ~AndroidGLRendererBackend() override;

    mbgl::gfx::RendererBackend& getImpl() override { return *this; }

    void updateViewPort() override;

    // Ensures the current context is not cleaned up when destroyed
    void markContextLost() override;

    void resizeFramebuffer(int width, int height) override;
    PremultipliedImage readFramebuffer() override;

    bool supportFreeThreadedUpload() override;
    std::shared_ptr<gl::UploadThreadContext> createUploadThreadContext() override;

    gfx::CustomPuckState getCurrentCustomPuckState() const override {
        return getCustomPuckState();
    }

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

private:
    void initFreeThreadedUpload();

private:
    int eglClientVersion = 0;
    EGLContext eglMainCtx = EGL_NO_CONTEXT;
    EGLDisplay eglDsply = EGL_NO_DISPLAY;
    EGLSurface eglSurf = EGL_NO_SURFACE;
    EGLConfig eglConfig;
    bool multiThreadedGpuResourceUpload = false;
};

class AndroidUploadThreadContext : public gl::UploadThreadContext {
public:
    AndroidUploadThreadContext(AndroidRendererBackend&, EGLDisplay, EGLConfig, EGLContext, int);
    ~AndroidUploadThreadContext() override;
    void createContext() override;
    void destroyContext() override;
    void bindContext() override;
    void unbindContext() override;

private:
    AndroidRendererBackend& backend;
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLConfig config;
    EGLContext mainContext = EGL_NO_CONTEXT;
    EGLContext sharedContext = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;
    int clientVersion = 0;
};

} // namespace android
} // namespace mbgl
