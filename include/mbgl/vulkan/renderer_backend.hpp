#pragma once

#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/context.hpp>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#define VMA_VULKAN_VERSION 1000000
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"

#ifndef NDEBUG
#define ENABLE_VULKAN_VALIDATION
// #define ENABLE_VULKAN_GPU_ASSISTED_VALIDATION
//  #define ENABLE_VMA_DEBUG
#endif

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
    const vk::PhysicalDevice& getPhysicalDevice() const { return physicalDevice; }
    const vk::UniqueDevice& getDevice() const { return device; }
    const vk::UniqueCommandPool& getCommandPool() const { return commandPool; }
    const vk::Queue& getGraphicsQueue() const { return graphicsQueue; }
    const vk::Queue& getPresentQueue() const { return presentQueue; }
    uint32_t getMaxFrames() const { return maxFrames; }
    const VmaAllocator& getAllocator() const { return allocator; }
    const vk::PhysicalDeviceProperties& getDeviceProperties() const { return physicalDeviceProperties; }
    const vk::PhysicalDeviceFeatures& getDeviceFeatures() const { return physicalDeviceFeatures; }
    int32_t getGraphicsQueueIndex() const { return graphicsQueueIndex; }
    int32_t getPresentQueueIndex() const { return presentQueueIndex; }

    template <typename T, typename = typename std::enable_if<vk::isVulkanHandleType<T>::value>>
    void setDebugName([[maybe_unused]] const T& object, [[maybe_unused]] const std::string& name) const {
#ifdef ENABLE_VULKAN_VALIDATION
        if (!debugUtilsEnabled) return;
        const uint64_t handle = reinterpret_cast<uint64_t>(static_cast<typename T::CType>(object));
        device->setDebugUtilsObjectNameEXT(vk::DebugUtilsObjectNameInfoEXT()
                                               .setObjectType(object.objectType)
                                               .setObjectHandle(handle)
                                               .setPObjectName(name.c_str()));
#endif
    }

    void beginDebugLabel(const vk::CommandBuffer& buffer, const char* name, const std::array<float, 4>& color) const;
    void endDebugLabel(const vk::CommandBuffer& buffer) const;
    void insertDebugLabel(const vk::CommandBuffer& buffer, const char* name) const;

    void startFrameCapture();
    void endFrameCapture();

protected:
    std::unique_ptr<gfx::Context> createContext() override;

    virtual std::vector<const char*> getLayers();
    virtual std::vector<const char*> getInstanceExtensions();
    virtual std::vector<const char*> getDeviceExtensions();
    std::vector<const char*> getDebugExtensions();

    void initInstance();
    void initDebug();
    void initSurface();
    void initDevice();
    void initAllocator();
    void initSwapchain();
    void initCommandPool();
    void initFrameCapture();

    void destroyResources();

protected:
    vk::DynamicLoader dynamicLoader;
    vk::UniqueInstance instance;
    vk::UniqueDebugUtilsMessengerEXT debugUtilsCallback;
    vk::UniqueDebugReportCallbackEXT debugReportCallback;

    vk::PhysicalDevice physicalDevice;
    vk::UniqueDevice device;
    vk::PhysicalDeviceProperties physicalDeviceProperties;
    vk::PhysicalDeviceFeatures physicalDeviceFeatures;

    int32_t graphicsQueueIndex = -1;
    int32_t presentQueueIndex = -1;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    vk::UniqueCommandPool commandPool;
    uint32_t maxFrames = 1;

    VmaAllocator allocator;

    bool debugUtilsEnabled{false};
};

} // namespace vulkan
} // namespace mbgl
