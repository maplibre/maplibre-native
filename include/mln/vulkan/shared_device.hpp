#pragma once

#include <mln/vulkan/renderer_backend.hpp>

namespace mln {
namespace vulkan {

// Instance, device and allocator that renderer backends can share instead of creating their own.
// The backends of a SharedDevice submit to the same queue: render with one of them at a time.
class SharedDevice {
public:
    SharedDevice();
    ~SharedDevice();

    SharedDevice(const SharedDevice&) = delete;
    SharedDevice& operator=(const SharedDevice&) = delete;

    // Used by the backends gfx::HeadlessBackend::Create() makes on the calling thread.
    static std::shared_ptr<SharedDevice> forCurrentThread();

private:
    friend class RendererBackend;

    DynamicLoader dynamicLoader;
    DispatchLoaderDynamic dispatcher;

    vk::Instance instance;
    vk::DebugUtilsMessengerEXT debugUtilsCallback;
    vk::DebugReportCallbackEXT debugReportCallback;
    bool debugUtilsEnabled{false};

    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceFeatures physicalDeviceFeatures;
    vk::Device device;
    int32_t graphicsQueueIndex = -1;
    int32_t presentQueueIndex = -1;

    VmaAllocator allocator{nullptr};
};

} // namespace vulkan
} // namespace mln
