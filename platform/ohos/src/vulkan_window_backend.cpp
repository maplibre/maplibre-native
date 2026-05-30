#include "vulkan_window_backend.hpp"

#include "native_window_utils.hpp"

#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/vulkan/context.hpp>

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace mbgl {
namespace ohos {
namespace {

constexpr const char* kOhosSurfaceExtensionName = "VK_OHOS_surface";
constexpr VkStructureType kOhosSurfaceCreateInfoType = static_cast<VkStructureType>(1000685000);

using VkSurfaceCreateFlagsOHOS = VkFlags;
struct VkSurfaceCreateInfoOHOS {
    VkStructureType sType;
    const void* pNext;
    VkSurfaceCreateFlagsOHOS flags;
    OHNativeWindow* window;
};

using PFN_vkCreateSurfaceOHOS = VkResult(VKAPI_PTR*)(VkInstance,
                                                     const VkSurfaceCreateInfoOHOS*,
                                                     const VkAllocationCallbacks*,
                                                     VkSurfaceKHR*);

std::string formatVulkanDiagnostic(const vulkan::RendererBackend& backend) {
    const auto& properties = backend.getDeviceProperties();
    const auto apiVersion = properties.apiVersion;
    std::ostringstream stream;
    stream << "Vulkan device name=\"" << properties.deviceName.data() << "\""
           << " api=" << VK_VERSION_MAJOR(apiVersion) << '.' << VK_VERSION_MINOR(apiVersion) << '.'
           << VK_VERSION_PATCH(apiVersion)
           << " vendor=0x" << std::hex << properties.vendorID
           << " device=0x" << properties.deviceID << std::dec
           << " graphicsQueue=" << backend.getGraphicsQueueIndex()
           << " presentQueue=" << backend.getPresentQueueIndex();
    return stream.str();
}

std::string formatVulkanLoaderDiagnostic(const vk::DispatchLoaderDynamic& dispatcher) {
    std::uint32_t apiVersion = VK_API_VERSION_1_0;
    const auto enumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
        dispatcher.vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
    if (enumerateInstanceVersion) {
        enumerateInstanceVersion(&apiVersion);
    }

    bool hasKhrSurface = false;
    bool hasOhosSurface = false;
    auto extensionCount = std::size_t{0};
    try {
        const auto extensions = vk::enumerateInstanceExtensionProperties(nullptr, dispatcher);
        extensionCount = extensions.size();
        hasKhrSurface = std::any_of(extensions.begin(), extensions.end(), [](const vk::ExtensionProperties& value) {
            return std::string_view(value.extensionName.data()) == VK_KHR_SURFACE_EXTENSION_NAME;
        });
        hasOhosSurface = std::any_of(extensions.begin(), extensions.end(), [](const vk::ExtensionProperties& value) {
            return std::string_view(value.extensionName.data()) == kOhosSurfaceExtensionName;
        });
    } catch (const std::exception& exception) {
        std::ostringstream stream;
        stream << "Vulkan loader api=" << VK_VERSION_MAJOR(apiVersion) << '.' << VK_VERSION_MINOR(apiVersion) << '.'
               << VK_VERSION_PATCH(apiVersion) << " extension enumeration failed: " << exception.what();
        return stream.str();
    }

    std::ostringstream stream;
    stream << "Vulkan loader api=" << VK_VERSION_MAJOR(apiVersion) << '.' << VK_VERSION_MINOR(apiVersion) << '.'
           << VK_VERSION_PATCH(apiVersion)
           << " instanceExtensions=" << extensionCount
           << " " << VK_KHR_SURFACE_EXTENSION_NAME << '=' << (hasKhrSurface ? "yes" : "no")
           << " " << kOhosSurfaceExtensionName << '=' << (hasOhosSurface ? "yes" : "no");
    return stream.str();
}

class VulkanWindowRenderableResource final : public vulkan::SurfaceRenderableResource {
public:
    explicit VulkanWindowRenderableResource(VulkanWindowBackend& backend_)
        : SurfaceRenderableResource(backend_) {}

    std::vector<const char*> getDeviceExtensions() override { return {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; }

    void createPlatformSurface() override {
        auto& backendImpl = static_cast<VulkanWindowBackend&>(backend);

        const auto instance = static_cast<VkInstance>(backendImpl.getInstance().get());
        const auto createSurface = reinterpret_cast<PFN_vkCreateSurfaceOHOS>(
            backendImpl.getDispatcher().vkGetInstanceProcAddr(instance, "vkCreateSurfaceOHOS"));
        if (!createSurface) {
            throw std::runtime_error("vkCreateSurfaceOHOS is not available");
        }

        const VkSurfaceCreateInfoOHOS createInfo{
            .sType = kOhosSurfaceCreateInfoType,
            .pNext = nullptr,
            .flags = 0,
            .window = backendImpl.getNativeWindow(),
        };

        VkSurfaceKHR rawSurface = VK_NULL_HANDLE;
        const auto result = createSurface(instance, &createInfo, nullptr, &rawSurface);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("vkCreateSurfaceOHOS failed with VkResult " + std::to_string(result));
        }

        surface = vk::UniqueSurfaceKHR(
            rawSurface,
            vk::ObjectDestroy<vk::Instance, vk::DispatchLoaderDynamic>(
                backendImpl.getInstance().get(), nullptr, backendImpl.getDispatcher()));
    }

    void bind() override {}
};

} // namespace

VulkanWindowBackend::VulkanWindowBackend(OHNativeWindow* window_, Size size_)
    : vulkan::RendererBackend(gfx::ContextMode::Unique),
      vulkan::Renderable(size_, std::make_unique<VulkanWindowRenderableResource>(*this)),
      window(window_) {
    MLN_TRACE_FUNC();

    if (window == nullptr) {
        throw std::invalid_argument("VulkanWindowBackend requires a native window");
    }

    const auto referenceResult = OH_NativeWindow_NativeObjectReference(window);
    if (referenceResult != 0) {
        throw std::runtime_error("OH_NativeWindow_NativeObjectReference failed");
    }

    try {
        if (!setNativeWindowBufferGeometry(window, size_)) {
            Log::Warning(Event::Render, "OH_NativeWindow_NativeWindowHandleOpt SET_BUFFER_GEOMETRY failed");
        }
        init();
        rendererDiagnostic = formatVulkanDiagnostic(*this);
        Log::Info(Event::Render, rendererDiagnostic);
    } catch (...) {
        OH_NativeWindow_NativeObjectUnreference(window);
        window = nullptr;
        throw;
    }
}

VulkanWindowBackend::~VulkanWindowBackend() {
    MLN_TRACE_FUNC();

    context.reset();
    if (window != nullptr) {
        OH_NativeWindow_NativeObjectUnreference(window);
        window = nullptr;
    }
}

void VulkanWindowBackend::setSize(Size size_) {
    size = size_;
    if (!setNativeWindowBufferGeometry(window, size)) {
        Log::Warning(Event::Render, "OH_NativeWindow_NativeWindowHandleOpt SET_BUFFER_GEOMETRY failed");
    }
    if (context) {
        static_cast<vulkan::Context&>(*context).requestSurfaceUpdate();
    }
}

void VulkanWindowBackend::initInstance() {
    const auto loaderDiagnostic = formatVulkanLoaderDiagnostic(dispatcher);
    Log::Info(Event::Render, loaderDiagnostic);

    try {
        vulkan::RendererBackend::initInstance();
    } catch (const std::exception& exception) {
        throw std::runtime_error(std::string(exception.what()) + " (" + loaderDiagnostic + ")");
    }
}

std::vector<const char*> VulkanWindowBackend::getInstanceExtensions() {
    auto extensions = vulkan::RendererBackend::getInstanceExtensions();
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(kOhosSurfaceExtensionName);
    return extensions;
}

} // namespace ohos
} // namespace mbgl
