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
    virtual ~RenderableResource() { destroyResources(); }

    virtual void createSurface() {}

    virtual void destroyResources() {
        // specific order
        swapchainFramebuffers.clear();
        renderPass.reset();
        swapchainImageViews.clear();
        swapchainImages.clear();
        swapchain.reset();
        surface.reset();

        depthAllocation.reset();
    }

    virtual std::vector<const char*> getDeviceExtensions() { return {}; }

public:
    const vk::UniqueRenderPass& getRenderPass() const { return renderPass; }
    const vk::Extent2D& getExtent() const { return extent; }

    virtual void swap() {
        // Renderable resources that require a swap function to be called
        // explicitly can override this method.
    }

protected:
    friend class RendererBackend;
    friend class RenderPass;
    friend class Context;

    vk::UniqueSurfaceKHR surface;
    vk::UniqueSwapchainKHR swapchain;

    UniqueImageAllocation depthAllocation;
    vk::Format depthFormat;

    std::vector<vk::Image> swapchainImages;
    std::vector<vk::UniqueImageView> swapchainImageViews;
    std::vector<vk::UniqueFramebuffer> swapchainFramebuffers;
    uint32_t acquiredImageIndex = 0;

    vk::Format colorFormat;
    vk::Extent2D extent;

    vk::UniqueRenderPass renderPass;
};

} // namespace vulkan
} // namespace mbgl
