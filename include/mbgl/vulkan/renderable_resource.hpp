#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/texture2d.hpp>

namespace mbgl {
namespace vulkan {

class Context;

class RenderableResource : public gfx::RenderableResource {
protected:
    explicit RenderableResource(RendererBackend& backend_)
        : backend(backend_) {}
    ~RenderableResource() override = default;

public:
    virtual void swap() {
        // Renderable resources that require a swap function to be called
        // explicitly can override this method.
    }

    const vk::Extent2D& getExtent() const { return extent; }
    const vk::UniqueRenderPass& getRenderPass() const { return renderPass; }
    virtual const vk::UniqueFramebuffer& getFramebuffer() const = 0;

protected:
    RendererBackend& backend;

    vk::Extent2D extent;
    vk::UniqueRenderPass renderPass;
};

class SurfaceRenderableResource : public RenderableResource {
protected:
    explicit SurfaceRenderableResource(RendererBackend& backend_)
        : RenderableResource(backend_) {}
    ~SurfaceRenderableResource() override;

    void initColor(uint32_t w, uint32_t h);
    void initSwapchain(uint32_t w, uint32_t h, vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo);

    void initDepthStencil();

    void swap() override;

public:
    virtual void createPlatformSurface() = 0;
    virtual std::vector<const char*> getDeviceExtensions() { return {}; }

    const vk::UniqueSurfaceKHR& getPlatformSurface() const { return surface; }
    const vk::UniqueSwapchainKHR& getSwapchain() const { return swapchain; }
    const vk::UniqueFramebuffer& getFramebuffer() const override;

    uint32_t getImageCount() const { return static_cast<uint32_t>(swapchainFramebuffers.size()); };
    uint32_t getAcquiredImageIndex() const { return acquiredImageIndex; };
    void setAcquiredImageIndex(uint32_t index) { acquiredImageIndex = index; };
    const vk::Image getAcquiredImage() const;

    void init(uint32_t w, uint32_t h);
    void recreateSwapchain();

protected:
    vk::UniqueSurfaceKHR surface;
    vk::UniqueSwapchainKHR swapchain;

    uint32_t acquiredImageIndex{0};

    // optional color textures when no surface is available
    std::vector<UniqueImageAllocation> colorAllocations;

    std::vector<vk::Image> swapchainImages;
    std::vector<vk::UniqueImageView> swapchainImageViews;
    std::vector<vk::UniqueFramebuffer> swapchainFramebuffers;
    vk::Format colorFormat{vk::Format::eUndefined};

    UniqueImageAllocation depthAllocation;
    vk::Format depthFormat{vk::Format::eUndefined};
};

} // namespace vulkan
} // namespace mbgl
