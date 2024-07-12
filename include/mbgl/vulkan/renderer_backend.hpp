#pragma once

#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/context.hpp>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#define VMA_VULKAN_VERSION 1000000
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"

#ifndef NDEBUG
#define ELABLE_VULKAN_VALIDATION
// #define ENABLE_VMA_DEBUG
#endif

namespace mbgl {

class ProgramParameters;

namespace vulkan {

struct BufferAllocation {
    const VmaAllocator& allocator;
    VkBuffer buffer{};
    VmaAllocation allocation{};
    void* mappedBuffer{};

    BufferAllocation() = delete;
    BufferAllocation(BufferAllocation&) = delete;
    BufferAllocation& operator=(const BufferAllocation& other) = delete;

    BufferAllocation(const VmaAllocator& allocator_)
        : allocator(allocator_) {}

    BufferAllocation(BufferAllocation&& other) noexcept
        : allocator(other.allocator),
          buffer(other.buffer),
          allocation(other.allocation),
          mappedBuffer(other.mappedBuffer) {
        other.buffer = nullptr;
        other.allocation = nullptr;
    }

    ~BufferAllocation() {
        if (mappedBuffer) vmaUnmapMemory(allocator, allocation);
        vmaDestroyBuffer(allocator, buffer, allocation);
    }
};

struct ImageAllocation {
    const VmaAllocator& allocator;
    VkImage image{};
    VmaAllocation allocation{};

    vk::UniqueImageView imageView{};

    ImageAllocation() = delete;
    ImageAllocation(ImageAllocation&) = delete;
    ImageAllocation& operator=(const ImageAllocation& other) = delete;

    ImageAllocation(const VmaAllocator& allocator_)
        : allocator(allocator_) {}

    ImageAllocation(ImageAllocation&& other) noexcept
        : allocator(other.allocator),
          image(other.image),
          allocation(other.allocation),
          imageView(std::move(other.imageView)) {
        other.image = nullptr;
        other.allocation = nullptr;
    }

    ~ImageAllocation() {
        imageView.reset();
        vmaDestroyImage(allocator, image, allocation);
    }
};

using UniqueBufferAllocation = std::unique_ptr<BufferAllocation>;
using UniqueImageAllocation = std::unique_ptr<ImageAllocation>;

using SharedBufferAllocation = std::shared_ptr<BufferAllocation>;
using SharedImageAllocation = std::shared_ptr<ImageAllocation>;

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
    int32_t getGraphicsQueueIndex() const { return graphicsQueueIndex; }
    int32_t getPresentQueueIndex() const { return presentQueueIndex; }

    template <typename T, typename = typename std::enable_if<vk::isVulkanHandleType<T>::value>>
    void setDebugName([[maybe_unused]] const T& object, [[maybe_unused]] const std::string& name) const {
#ifdef ELABLE_VULKAN_VALIDATION
        const uint64_t handle = reinterpret_cast<uint64_t>(static_cast<typename T::CType>(object));
        device->setDebugUtilsObjectNameEXT(vk::DebugUtilsObjectNameInfoEXT()
                                               .setObjectType(object.objectType)
                                               .setObjectHandle(handle)
                                               .setPObjectName(name.c_str()));
#endif
    }

    void startFrameCapture();
    void endFrameCapture();

protected:
    std::unique_ptr<gfx::Context> createContext() override;

    virtual std::vector<const char*> getLayers();
    virtual std::vector<const char*> getInstanceExtensions();
    virtual std::vector<const char*> getDeviceExtensions();

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
    vk::UniqueDebugUtilsMessengerEXT debugCallback;

    vk::PhysicalDevice physicalDevice;
    vk::UniqueDevice device;
    vk::PhysicalDeviceProperties physicalDeviceProperties;

    int32_t graphicsQueueIndex = -1;
    int32_t presentQueueIndex = -1;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    vk::UniqueCommandPool commandPool;
    uint32_t maxFrames = 1;

    VmaAllocator allocator;
};

} // namespace vulkan
} // namespace mbgl
