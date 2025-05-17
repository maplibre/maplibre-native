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
    explicit SurfaceRenderableResource(RendererBackend& backend_, vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo)
        : RenderableResource(backend_),
          presentMode(mode) {}
    ~SurfaceRenderableResource() override;

    void initColor(uint32_t w, uint32_t h);
    void initSwapchain(uint32_t w, uint32_t h);

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
    const vk::Semaphore& getAcquiredSemaphore() const;

    bool hasSurfaceTransformSupport() const;
    bool didSurfaceTransformUpdate() const;

    // rotation needed to align framebuffer contents with device surface
    float getRotation();

    void setSurfaceTransformPollingInterval(int32_t value) { surfaceTransformPollingInterval = value; }
    int32_t getSurfaceTransformPollingInterval() const { return surfaceTransformPollingInterval; }

    void init(uint32_t w, uint32_t h);
    void recreateSwapchain();

protected:
    vk::UniqueSurfaceKHR surface;
    vk::UniqueSwapchainKHR swapchain;
    vk::PresentModeKHR presentMode;

    vk::SurfaceCapabilitiesKHR capabilities;

    uint32_t acquiredImageIndex{0};

    // optional color textures when no surface is available
    std::vector<UniqueImageAllocation> colorAllocations;

    std::vector<vk::Image> swapchainImages;
    std::vector<vk::UniqueImageView> swapchainImageViews;
    std::vector<vk::UniqueFramebuffer> swapchainFramebuffers;
    std::vector<vk::UniqueSemaphore> swapchainSemaphores;
    vk::Format colorFormat{vk::Format::eUndefined};

    UniqueImageAllocation depthAllocation;
    vk::Format depthFormat{vk::Format::eUndefined};

    int32_t surfaceTransformPollingInterval{-1};
};

class Renderable : public gfx::Renderable {
protected:
    Renderable(const Size size_, std::unique_ptr<gfx::RenderableResource> resource_)
        : gfx::Renderable(size_, std::move(resource_)) {}
    virtual ~Renderable() override = default;

public:
    void setSize(const Size& size_) { size = size_; }
};

} // namespace vulkan
} // namespace mbgl
