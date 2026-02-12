#pragma once

#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
#include "android_renderer_backend.hpp"
#include <android/native_window.h>

#if MLN_WEBGPU_IMPL_WGPU
#include <wgpu.h>
#endif

namespace mbgl {
namespace android {

class AndroidWebGPURendererBackend : public AndroidRendererBackend,
                                     public webgpu::RendererBackend,
                                     public gfx::Renderable {
public:
    AndroidWebGPURendererBackend(ANativeWindow*);
    ~AndroidWebGPURendererBackend() override;

    ANativeWindow* getWindow() { return window; }
    mbgl::gfx::RendererBackend& getImpl() override { return *this; }

    void resizeFramebuffer(int width, int height) override;
    PremultipliedImage readFramebuffer() override;

public:
    mbgl::gfx::Renderable& getDefaultRenderable() override { return *this; }

    void* getCurrentTextureView() override;
    void* getDepthStencilView() override;
    mbgl::Size getFramebufferSize() const override;

    void presentSurface();

protected:
    void activate() override {}
    void deactivate() override {}

private:
    void createSurface();
    void configureSurface(uint32_t width, uint32_t height);
    void createDepthStencilTexture(uint32_t width, uint32_t height);

    class Impl;
    std::unique_ptr<Impl> impl;
    ANativeWindow* window;
};

} // namespace android
} // namespace mbgl
