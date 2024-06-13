#pragma once

#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/context.hpp>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace mbgl {

class ProgramParameters;

namespace vulkan {

class RendererBackend : public gfx::RendererBackend {
public:
    RendererBackend(gfx::ContextMode);
    ~RendererBackend() override;

    /// One-time shader initialization
    void initShaders(gfx::ShaderRegistry&, const ProgramParameters& programParameters) override;
    void init();

    const vk::UniqueInstance& getInstance() const { return instance; }
    const vk::UniqueDevice& getDevice() const { return device; }
    const vk::UniqueCommandPool& getCommandPool() const { return commandPool; }
    const vk::Queue& getGraphicsQueue() const { return graphicsQueue; }
    const vk::Queue& getPresentQueue() const { return presentQueue; }
    const uint32_t getMaxFrames() const { return maxFrames; }

protected:
    std::unique_ptr<gfx::Context> createContext() override;


    virtual std::vector<const char*> getLayers();
    virtual std::vector<const char*> getInstanceExtensions();
    virtual std::vector<const char*> getDeviceExtensions();

    void createDebugCallback();

    void initInstance();
    void initSurface();
    void initDevice();
    void initAllocator();
    void initSwapchain();
    void initCommandPool();

protected:
    
    vk::DynamicLoader dynamicLoader;
    vk::UniqueInstance instance;
    vk::UniqueDebugUtilsMessengerEXT debugCallback;

    vk::PhysicalDevice physicalDevice;
    vk::UniqueDevice device;

    int32_t graphicsQueueIndex = -1;
    int32_t presentQueueIndex = -1;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    vk::UniqueCommandPool commandPool;
    uint32_t maxFrames = 2;
};

} // namespace vulkan
} // namespace mbgl
