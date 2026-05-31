#pragma once

#include "window_backend.hpp"

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gl/renderer_backend.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/size.hpp>

#include <EGL/egl.h>
#include <native_window/external_window.h>

#include <cstdint>
#include <memory>
#include <string>

namespace mbgl {
namespace ohos {

class EGLWindowBackend final : public WindowBackend, public gl::RendererBackend, public gfx::Renderable {
public:
    EGLWindowBackend(OHNativeWindow*, Size);
    ~EGLWindowBackend() override;

    EGLWindowBackend(const EGLWindowBackend&) = delete;
    EGLWindowBackend& operator=(const EGLWindowBackend&) = delete;

    gfx::RendererBackend& getRendererBackend() override { return *this; }
    gfx::Renderable& getDefaultRenderable() override { return *this; }

    OHNativeWindow* getNativeWindow() const override { return window; }
    std::int32_t getGlesContextClientVersion() const override { return contextClientVersion; }
    void setSize(Size) override;
    void swap();
    PremultipliedImage readStillImage();

protected:
    void activate() override;
    void deactivate() override;

    gl::ProcAddress getExtensionFunctionPointer(const char*) override;
    void updateAssumedState() override;

private:
    class DisplayConfig;

    std::shared_ptr<const DisplayConfig> displayConfig;
    EGLContext eglContext = EGL_NO_CONTEXT;
    EGLSurface eglSurface = EGL_NO_SURFACE;
    OHNativeWindow* window = nullptr;
    std::int32_t contextClientVersion = 0;
    std::string eglConfigDiagnostic;
    std::string framebufferDiagnostic;
};

} // namespace ohos
} // namespace mbgl
