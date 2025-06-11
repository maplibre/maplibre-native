#pragma once

#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include "android_renderer_backend.hpp"
#include <android/native_window.h>

namespace mbgl {
namespace android {

class AndroidVulkanRendererBackend : public AndroidRendererBackend,
                                     public vulkan::RendererBackend,
                                     public vulkan::Renderable {
public:
    AndroidVulkanRendererBackend(ANativeWindow*);
    ~AndroidVulkanRendererBackend() override;

    ANativeWindow* getWindow() { return window; }
    mbgl::gfx::RendererBackend& getImpl() override { return *this; }

    std::vector<const char*> getInstanceExtensions() override;

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

protected:
    ANativeWindow* window;
};

} // namespace android
} // namespace mbgl
