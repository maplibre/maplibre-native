#include "glfw_vulkan_backend.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/context.hpp>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef USE_SHARED_VK_CONTEXT

// An example of an external vkInstance/vkDevice provider.
// It uses the first available device and queue
class VkContext {
public:
    VkContext() = default;
    ~VkContext() {
        device.reset();
        instance.reset();
    }

    static VkContext& shared() {
        static std::unique_ptr<VkContext> context;
        if (!context) {
            context = std::make_unique<VkContext>();
        }

        return *context;
    }

    vk::Instance getInstance() {
        if (instance) {
            return instance.get();
        }

        dispatcher.init(dynamicLoader);

#ifdef __APPLE__
        vk::InstanceCreateFlags instanceFlags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#else
        vk::InstanceCreateFlags instanceFlags = {};
#endif

        const std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
        std::vector<const char*> extensions = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#ifdef __APPLE__
            "VK_KHR_portability_enumeration",
#endif
        };

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::copy(glfwExtensions, glfwExtensions + glfwExtensionCount, std::back_inserter(extensions));

        vk::ApplicationInfo appInfo("maplibre-native", 1, "maplibre-native", 1, VK_API_VERSION_1_0);
        const auto createInfo = vk::InstanceCreateInfo(instanceFlags)
                                    .setPApplicationInfo(&appInfo)
                                    .setPEnabledExtensionNames(extensions)
                                    .setPEnabledLayerNames(layers);

        instance = vk::createInstanceUnique(createInfo, nullptr, dispatcher);
        dispatcher.init(instance.get());

        return instance.get();
    }

    vk::PhysicalDevice getPhysicalDevice() {
        if (!physicalDevice) {
            physicalDevice = instance->enumeratePhysicalDevices(dispatcher)[0];
        }

        return physicalDevice;
    }

    vk::Device getDevice() {
        if (device) {
            return device.get();
        }

        const std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
            "VK_KHR_portability_subset",
#endif
        };

        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo(vk::DeviceQueueCreateFlags(), getQueueIndex(), 1, &queuePriority);

        auto createInfo = vk::DeviceCreateInfo()
                              .setQueueCreateInfos(queueCreateInfo)
                              .setPEnabledExtensionNames(extensions)
                              .setPEnabledLayerNames(layers);

        device = physicalDevice.createDeviceUnique(createInfo, nullptr, dispatcher);
        dispatcher.init(device.get());

        return device.get();
    }

    uint32_t getQueueIndex() { return graphicsQueueIndex; }

private:
    vk::DynamicLoader dynamicLoader;
    vk::DispatchLoaderDynamic dispatcher;

    vk::UniqueInstance instance;
    vk::PhysicalDevice physicalDevice;
    vk::UniqueDevice device;
    uint32_t graphicsQueueIndex = 0;
};

#endif

class GLFWVulkanRenderableResource final : public mbgl::vulkan::SurfaceRenderableResource {
public:
    explicit GLFWVulkanRenderableResource(GLFWVulkanBackend& backend_, bool capFrameRate)
        : SurfaceRenderableResource(backend_,
                                    capFrameRate ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate) {}

    std::vector<const char*> getDeviceExtensions() override { return {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; }

    void createPlatformSurface() override {
        auto& backendImpl = static_cast<GLFWVulkanBackend&>(backend);

        VkSurfaceKHR surface_;

        VkResult result = glfwCreateWindowSurface(
            backendImpl.getInstance().get(), backendImpl.getWindow(), nullptr, &surface_);
        if (result != VK_SUCCESS) throw std::runtime_error("Failed to create glfw window surface");

        surface = vk::UniqueSurfaceKHR(surface_,
                                       vk::ObjectDestroy<vk::Instance, vk::DispatchLoaderDynamic>(
                                           backendImpl.getInstance().get(), nullptr, backendImpl.getDispatcher()));
    }

    void bind() override {}
};

GLFWVulkanBackend::GLFWVulkanBackend(GLFWwindow* window_, const bool capFrameRate)
    : mbgl::vulkan::RendererBackend(mbgl::gfx::ContextMode::Unique),
      mbgl::vulkan::Renderable(
          [window_] {
              int fbWidth;
              int fbHeight;
              glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
              return mbgl::Size{static_cast<uint32_t>(fbWidth), static_cast<uint32_t>(fbHeight)};
          }(),
          std::make_unique<GLFWVulkanRenderableResource>(*this, capFrameRate)),
      window(window_) {
    init();
}

GLFWVulkanBackend::~GLFWVulkanBackend() {
    context.reset();
}

mbgl::Size GLFWVulkanBackend::getSize() const {
    return size;
}

void GLFWVulkanBackend::setSize(const mbgl::Size newSize) {
    size = newSize;

    auto& contextImpl = static_cast<mbgl::vulkan::Context&>(*context);
    contextImpl.requestSurfaceUpdate();
}

std::vector<const char*> GLFWVulkanBackend::getInstanceExtensions() {
    auto extensions = mbgl::vulkan::RendererBackend::getInstanceExtensions();

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::copy(glfwExtensions, glfwExtensions + glfwExtensionCount, std::back_inserter(extensions));
    return extensions;
}

#ifdef USE_SHARED_VK_CONTEXT

void GLFWVulkanBackend::initInstance() {
    // tell the backend to keep the objects alive
    usingSharedContext = true;

    // this method is required to set `instance` to a valid vkInstance
    instance = vk::UniqueInstance(VkContext::shared().getInstance(),
                                  vk::ObjectDestroy<vk::NoParent, vk::DispatchLoaderDynamic>(nullptr, dispatcher));

    // debug builds also require:
    // - enabling either `VK_EXT_debug_utils` (and updating `debugUtilsEnabled`) or `VK_EXT_debug_report` extension
    // - or passing the values returned by `getDebugExtensions()` as enabled extensions during instance creation
    debugUtilsEnabled = true;
}

void GLFWVulkanBackend::initDevice() {
    // this method is required to populate:
    // `physicalDevice`
    // `device`
    // `graphicsQueueIndex`/`presentQueueIndex`
    // `physicalDeviceFeatures` - enabled features (optional)

    physicalDevice = VkContext::shared().getPhysicalDevice();
    device = vk::UniqueDevice(VkContext::shared().getDevice(),
                              vk::ObjectDestroy<vk::NoParent, vk::DispatchLoaderDynamic>(nullptr, dispatcher));

    graphicsQueueIndex = presentQueueIndex = VkContext::shared().getQueueIndex();
    physicalDeviceFeatures = vk::PhysicalDeviceFeatures();
}

#endif

namespace mbgl {
namespace gfx {

template <>
std::unique_ptr<GLFWBackend> Backend::Create<mbgl::gfx::Backend::Type::Vulkan>(GLFWwindow* window, bool capFrameRate) {
    return std::make_unique<GLFWVulkanBackend>(window, capFrameRate);
}

} // namespace gfx
} // namespace mbgl
