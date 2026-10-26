#pragma once

#include <mln/vulkan/renderer_backend.hpp>

namespace mln {
namespace vulkan {

/// A Vulkan instance, device and memory allocator that several renderer backends use instead of
/// creating their own: with lavapipe, a device is a whole software rasterizer with its own threads
/// and memory, which a process that keeps many headless maps does not need once per map.
///
/// The first backend given a `SharedDevice` creates its objects; the others reuse them. They are
/// destroyed with the `SharedDevice`, once the last backend using it and its other owners are gone.
///
/// The backends of a `SharedDevice` submit to the same queue, which Vulkan requires to be
/// externally synchronized: use them one at a time, for example by creating and using them all on
/// one thread.
class SharedDevice {
public:
    SharedDevice();
    ~SharedDevice();

    SharedDevice(const SharedDevice&) = delete;
    SharedDevice& operator=(const SharedDevice&) = delete;

private:
    friend class RendererBackend;

    // Keeps the Vulkan library loaded while the objects below are alive.
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
