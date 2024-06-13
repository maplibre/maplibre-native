#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>

#include <cstdlib>

namespace mbgl {
namespace vulkan {

class RendererBackend;

class RenderableResource : public gfx::RenderableResource {
protected:
    explicit RenderableResource() = default;

    virtual void createSurface() {}
    virtual std::vector<const char*> getDeviceExtensions() { return {}; }

public:
    virtual void swap() {
        // Renderable resources that require a swap function to be called
        // explicitly can override this method.
    }

protected:
    friend class RendererBackend;
    friend class RenderPass;
    friend class Context;

    vk::SurfaceKHR surface;
    vk::UniqueSwapchainKHR swapchain;

    std::vector<vk::Image> swapchainImages;
    std::vector<vk::UniqueImageView> swapchainImageViews;
    std::vector<vk::UniqueFramebuffer> swapchainFramebuffers;
    uint32_t acquiredImageIndex = 0;

    vk::Format format;
    vk::Extent2D extent;

    vk::UniqueRenderPass renderPass;
};

} // namespace vulkan
} // namespace mbgl
