#pragma once

#include <mln/vulkan/renderable_resource.hpp>
#include <mln/vulkan/renderer_backend.hpp>
#include "android_renderer_backend.hpp"
#include <android/native_window.h>

namespace mln {
namespace android {

class AndroidVulkanRendererBackend : public AndroidRendererBackend,
                                     public vulkan::RendererBackend,
                                     public vulkan::Renderable {
public:
    AndroidVulkanRendererBackend(ANativeWindow*);
    ~AndroidVulkanRendererBackend() override;

    ANativeWindow* getWindow() { return window; }
    bool createSurface(ANativeWindow* window) override;
    void destroySurface() override;

    mln::gfx::RendererBackend& getImpl() override { return *this; }

    std::vector<const char*> getInstanceExtensions() override;

    void resizeFramebuffer(int width, int height) override;
    void enableFramebufferRead(bool value) override;
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

protected:
    ANativeWindow* window;

    MBGL_STORE_THREAD(tid);
};

} // namespace android
} // namespace mln
