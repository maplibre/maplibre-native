#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include "android_renderer_backend.hpp"
#include <android/native_window.h>

namespace mbgl {
namespace android {

class AndroidVulkanRendererBackend : public AndroidRendererBackend,
                                     public vulkan::RendererBackend,
                                     public mbgl::gfx::Renderable {
public:
    AndroidVulkanRendererBackend(ANativeWindow*);
    ~AndroidVulkanRendererBackend() override;

    ANativeWindow* getWindow() { return window; }
    mbgl::gfx::RendererBackend& getImpl() override { return *this; }

    std::vector<const char*> getInstanceExtensions() override;

    void resizeFramebuffer(int width, int height) override;

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
