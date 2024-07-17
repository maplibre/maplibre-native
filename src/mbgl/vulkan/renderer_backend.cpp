#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/logging.hpp>

#include <mbgl/shaders/vulkan/shader_group.hpp>
#include <mbgl/shaders/vulkan/background.hpp>
#include <mbgl/shaders/vulkan/circle.hpp>
#include <mbgl/shaders/vulkan/clipping_mask.hpp>
#include <mbgl/shaders/vulkan/collision.hpp>
#include <mbgl/shaders/vulkan/debug.hpp>
#include <mbgl/shaders/vulkan/fill.hpp>
#include <mbgl/shaders/vulkan/heatmap.hpp>
#include <mbgl/shaders/vulkan/hillshade.hpp>
#include <mbgl/shaders/vulkan/line.hpp>
#include <mbgl/shaders/vulkan/raster.hpp>
#include <mbgl/shaders/vulkan/symbol.hpp>

#include <cassert>
#include <string>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#ifdef ENABLE_VMA_DEBUG

#define VMA_DEBUG_MARGIN 32
#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1

#define VMA_DEBUG_LOG_FORMAT(format, ...)
#define VMA_LEAK_LOG_FORMAT(format, ...)
#define VMA_DEBUG_LOG_FORMAT(format, ...)           \
{                                                   \
    char buffer[4096];                              \
    sprintf(buffer, format, __VA_ARGS__);           \
    mbgl::Log::Info(mbgl::Event::Render, buffer);   \
}

#endif

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#ifdef _WIN32

#ifndef NDEBUG
// #define ENABLE_RENDERDOC_FRAME_CAPTURE
#include <windows.h>
#endif

#endif

#ifdef ENABLE_RENDERDOC_FRAME_CAPTURE
#include "renderdoc_app.h"
static RENDERDOC_API_1_1_2* g_rdoc_api = nullptr;
#endif

namespace mbgl {
namespace vulkan {

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_),
      allocator(nullptr) {}

RendererBackend::~RendererBackend() {
    destroyResources();
}

std::unique_ptr<gfx::Context> RendererBackend::createContext() {
    return std::make_unique<vulkan::Context>(*this);
}

std::vector<const char*> RendererBackend::getLayers() {
    return {
#ifdef ELABLE_VULKAN_VALIDATION
        "VK_LAYER_KHRONOS_validation"
#endif
    };
}

std::vector<const char*> RendererBackend::getInstanceExtensions() {
    return {
#ifdef ELABLE_VULKAN_VALIDATION
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };
}

std::vector<const char*> RendererBackend::getDeviceExtensions() {
    return getDefaultRenderable().getResource<SurfaceRenderableResource>().getDeviceExtensions();
}

void RendererBackend::initFrameCapture() {
#ifdef ENABLE_RENDERDOC_FRAME_CAPTURE

#ifdef _WIN32
    if (HMODULE mod = GetModuleHandleA("renderdoc.dll")) {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&g_rdoc_api);
        assert(ret == 1);
    }
#endif

#endif
}

void RendererBackend::startFrameCapture() {
#ifdef ENABLE_RENDERDOC_FRAME_CAPTURE
    if (g_rdoc_api) {
        g_rdoc_api->StartFrameCapture(nullptr, nullptr);
    }
#endif
}

void RendererBackend::endFrameCapture() {
#ifdef ENABLE_RENDERDOC_FRAME_CAPTURE
    if (g_rdoc_api) {
        g_rdoc_api->EndFrameCapture(nullptr, nullptr);
    }
#endif
}

template <typename T, typename F>
static bool checkAvailability(const std::vector<T>& availableValues,
                              const std::vector<const char*>& requiredValues,
                              const F& getter) {
    for (const auto& requiredValue : requiredValues) {
        bool found = false;
        for (const auto& availableValue : availableValues) {
            if (strcmp(requiredValue, getter(availableValue)) == 0) {
                found = true;
                break;
            }
        }

        if (!found) return false;
    }

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT,
                                                      const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                      void*) {
    EventSeverity mbglSeverity = EventSeverity::Debug;

    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            mbglSeverity = EventSeverity::Debug;
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            mbglSeverity = EventSeverity::Info;
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            mbglSeverity = EventSeverity::Warning;
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            mbglSeverity = EventSeverity::Error;
            break;

        default:
            return VK_FALSE;
    }

    mbgl::Log::Record(mbglSeverity, mbgl::Event::Render, callbackData->pMessage);

    return VK_FALSE;
}

void RendererBackend::initDebug() {
    const vk::DebugUtilsMessageSeverityFlagsEXT severity = vk::DebugUtilsMessageSeverityFlagsEXT() |
                                                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    const vk::DebugUtilsMessageTypeFlagsEXT type = vk::DebugUtilsMessageTypeFlagsEXT() |
                                                   vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                   vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                   vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                   vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding;

    const auto createInfo =
        vk::DebugUtilsMessengerCreateInfoEXT().setMessageSeverity(severity).setMessageType(type).setPfnUserCallback(
            vkDebugCallback);

    debugCallback = instance->createDebugUtilsMessengerEXTUnique(createInfo);

    if (!debugCallback) mbgl::Log::Error(mbgl::Event::Render, "Failed to register Vulkan debug callback");
}

void RendererBackend::init() {
    initFrameCapture();
    initInstance();
    initSurface();
    initDevice();
    initAllocator();
    initSwapchain();
    initCommandPool();
}

void RendererBackend::initInstance() {
    // initialize minimal set of function pointers
    PFN_vkGetInstanceProcAddr getInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>(
        "vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(getInstanceProcAddr);

    // Vulkan 1.1 on Android is supported on 71% of devices (compared to 1.3 with 6%) as of April 23 2024
    // https://vulkan.gpuinfo.org/
    const vk::ApplicationInfo appInfo("maplibre-native", 1, "maplibre-native", VK_API_VERSION_1_0);
    vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(), &appInfo);

    const auto& layers = getLayers();

    bool layersAvailable = checkAvailability(vk::enumerateInstanceLayerProperties(),
                                             layers,
                                             [](const vk::LayerProperties& value) { return value.layerName.data(); });

    if (layersAvailable) {
        createInfo.setPEnabledLayerNames(layers);
    } else {
#ifndef NDEBUG
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan layers not found");
#endif
    }

    const auto& extensions = getInstanceExtensions();

    bool extensionsAvailable = checkAvailability(
        vk::enumerateInstanceExtensionProperties(), extensions, [](const vk::ExtensionProperties& value) {
            return value.extensionName.data();
        });

    if (extensionsAvailable) {
        createInfo.setPEnabledExtensionNames(extensions);
    } else {
#ifndef NDEBUG
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan extensions not found");
#endif
    }

    instance = vk::createInstanceUnique(createInfo);

    // initialize function pointers for instance
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

#ifdef ELABLE_VULKAN_VALIDATION
    // enable validation layer callback
    initDebug();
#endif
}

void RendererBackend::initSurface() {
    getDefaultRenderable().getResource<SurfaceRenderableResource>().createPlatformSurface();
}

void RendererBackend::initAllocator() {
    VmaVulkanFunctions functions = {};

    functions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

    functions.vkGetPhysicalDeviceProperties = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceProperties;
    functions.vkGetPhysicalDeviceMemoryProperties = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceMemoryProperties;
    functions.vkAllocateMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkAllocateMemory;
    functions.vkFreeMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkFreeMemory;
    functions.vkMapMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkMapMemory;
    functions.vkUnmapMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkUnmapMemory;
    functions.vkFlushMappedMemoryRanges = VULKAN_HPP_DEFAULT_DISPATCHER.vkFlushMappedMemoryRanges;
    functions.vkInvalidateMappedMemoryRanges = VULKAN_HPP_DEFAULT_DISPATCHER.vkInvalidateMappedMemoryRanges;
    functions.vkBindBufferMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindBufferMemory;
    functions.vkBindImageMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindImageMemory;
    functions.vkGetBufferMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetBufferMemoryRequirements;
    functions.vkGetImageMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetImageMemoryRequirements;
    functions.vkCreateBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkCreateBuffer;
    functions.vkDestroyBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkDestroyBuffer;
    functions.vkCreateImage = VULKAN_HPP_DEFAULT_DISPATCHER.vkCreateImage;
    functions.vkDestroyImage = VULKAN_HPP_DEFAULT_DISPATCHER.vkDestroyImage;
    functions.vkCmdCopyBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdCopyBuffer;
    functions.vkGetBufferMemoryRequirements2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetBufferMemoryRequirements2KHR;
    functions.vkGetImageMemoryRequirements2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetImageMemoryRequirements2KHR;
    functions.vkBindBufferMemory2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindBufferMemory2KHR;
    functions.vkBindImageMemory2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindImageMemory2KHR;
    functions.vkGetPhysicalDeviceMemoryProperties2KHR =
        VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceMemoryProperties2KHR;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};

    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device.get();
    allocatorCreateInfo.instance = instance.get();
    allocatorCreateInfo.pVulkanFunctions = &functions;

    VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    if (result != VK_SUCCESS) {
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan allocator init failed");
    }
}

void RendererBackend::initDevice() {
    const auto& extensions = getDeviceExtensions();
    const auto& layers = getLayers();
    const auto& surface = getDefaultRenderable().getResource<SurfaceRenderableResource>().getPlatformSurface().get();

    const auto& isPhysicalDeviceCompatible = [&](const vk::PhysicalDevice& candidate) -> bool {
        bool extensionsAvailable = checkAvailability(
            candidate.enumerateDeviceExtensionProperties(), extensions, [](const vk::ExtensionProperties& value) {
                return value.extensionName.data();
            });

        if (!extensionsAvailable) return false;

        graphicsQueueIndex = -1;
        presentQueueIndex = -1;

        const auto& queues = candidate.getQueueFamilyProperties();

        for (auto i = 0u; i < queues.size(); ++i) {
            const auto& queue = queues[i];

            if (queue.queueCount == 0) continue;

            if (queue.queueFlags & vk::QueueFlagBits::eGraphics) graphicsQueueIndex = i;

            if (surface && candidate.getSurfaceSupportKHR(i, surface)) presentQueueIndex = i;

            if (graphicsQueueIndex != -1 && (!surface || presentQueueIndex != -1)) break;
        }

        if (graphicsQueueIndex == -1 || (surface && presentQueueIndex == -1)) return false;

        if (surface) {
            if (candidate.getSurfaceFormatsKHR(surface).empty()) return false;
            if (candidate.getSurfacePresentModesKHR(surface).empty()) return false;
        }

        return true;
    };

    const auto& pickPhysicalDevice = [&]() {
        const auto& physicalDevices = instance->enumeratePhysicalDevices();
        if (physicalDevices.empty()) throw std::runtime_error("No Vulkan compatible GPU found");

        for (const auto& candidate : physicalDevices) {
            if (isPhysicalDeviceCompatible(candidate)) {
                physicalDevice = candidate;
                break;
            }
        }

        physicalDeviceProperties = physicalDevice.getProperties();

        if (!physicalDevice) throw std::runtime_error("No suitable GPU found");
    };

    pickPhysicalDevice();

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;

    queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags(), graphicsQueueIndex, 1, &queuePriority);

    if (surface && graphicsQueueIndex != presentQueueIndex)
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags(), presentQueueIndex, 1, &queuePriority);

    const auto& supportedDeviceFeatures = physicalDevice.getFeatures();

    vk::PhysicalDeviceFeatures enabledDeviceFeatures;

    if (supportedDeviceFeatures.wideLines) {
        enabledDeviceFeatures.setWideLines(true);

        // more wideLines info
        // physicalDeviceProperties.limits.lineWidthRange;
        // physicalDeviceProperties.limits.lineWidthGranularity;
    } else {
        mbgl::Log::Error(mbgl::Event::Render, "Wide line support not available");
    }

    auto createInfo = vk::DeviceCreateInfo()
                          .setQueueCreateInfos(queueCreateInfos)
                          .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
                          .setPpEnabledExtensionNames(extensions.data())
                          .setPEnabledFeatures(&enabledDeviceFeatures);

    // this is not needed for newer implementations
    createInfo.setPEnabledLayerNames(layers);

    device = physicalDevice.createDeviceUnique(createInfo);

    // optional function pointer specialization for device
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());

    graphicsQueue = device->getQueue(graphicsQueueIndex, 0);
    if (presentQueueIndex != -1) presentQueue = device->getQueue(presentQueueIndex, 0);
}

void RendererBackend::initSwapchain() {
    const auto& renderable = getDefaultRenderable();
    auto& renderableResource = renderable.getResource<SurfaceRenderableResource>();
    const auto& size = renderable.getSize();
    renderableResource.init(size.width, size.height);

    if (renderableResource.getPlatformSurface()) {
        maxFrames = renderableResource.getImageCount();
    }
}

void RendererBackend::initCommandPool() {
    const vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsQueueIndex);
    commandPool = device->createCommandPoolUnique(createInfo);
}

void RendererBackend::destroyResources() {
    device->waitIdle();

    context.reset();
    commandPool.reset();

    vmaDestroyAllocator(allocator);

    device.reset();

    // destroy this last so we have cleanup validation
    debugCallback.reset();
    instance.reset();
}

/// @brief Register a list of types with a shader registry instance
/// @tparam ...ShaderID Pack of BuiltIn:: shader IDs
/// @param registry A shader registry instance
/// @param programParameters ProgramParameters used to initialize each instance
template <shaders::BuiltIn... ShaderID>
void registerTypes(gfx::ShaderRegistry& registry, const ProgramParameters& programParameters) {
    /// The following fold expression will create a shader for every type
    /// in the parameter pack and register it with the shader registry.

    // Registration calls are wrapped in a lambda that throws on registration
    // failure, we shouldn't expect registration to faill unless the shader
    // registry instance provided already has conflicting programs present.
    (
        [&]() {
            using namespace std::string_literals;
            using ShaderClass = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Vulkan>;
            auto group = std::make_shared<ShaderGroup<ShaderID>>(programParameters);
            if (!registry.registerShaderGroup(std::move(group), ShaderClass::name)) {
                assert(!"duplicate shader group");
                throw std::runtime_error("Failed to register "s + ShaderClass::name + " with shader registry!");
            }
        }(),
        ...);
}

void RendererBackend::initShaders(gfx::ShaderRegistry& shaders, const ProgramParameters& programParameters) {
    registerTypes<shaders::BuiltIn::BackgroundShader,
                  shaders::BuiltIn::BackgroundPatternShader,
                  shaders::BuiltIn::CircleShader,
                  shaders::BuiltIn::ClippingMaskProgram,
                  shaders::BuiltIn::CollisionBoxShader,
                  shaders::BuiltIn::CollisionCircleShader,
                  shaders::BuiltIn::CustomSymbolIconShader,
                  shaders::BuiltIn::DebugShader,
                  shaders::BuiltIn::FillShader,
                  shaders::BuiltIn::FillOutlineShader,
                  shaders::BuiltIn::FillPatternShader,
                  shaders::BuiltIn::FillOutlinePatternShader,
                  shaders::BuiltIn::FillOutlineTriangulatedShader,
                  shaders::BuiltIn::FillExtrusionShader,
                  shaders::BuiltIn::FillExtrusionPatternShader,
                  shaders::BuiltIn::HeatmapShader,
                  shaders::BuiltIn::HeatmapTextureShader,
                  shaders::BuiltIn::HillshadeShader,
                  shaders::BuiltIn::HillshadePrepareShader,
                  shaders::BuiltIn::LineShader,
                  shaders::BuiltIn::LineGradientShader,
                  shaders::BuiltIn::LineSDFShader,
                  shaders::BuiltIn::LinePatternShader,
                  shaders::BuiltIn::RasterShader,
                  shaders::BuiltIn::SymbolIconShader,
                  shaders::BuiltIn::SymbolSDFIconShader,
                  shaders::BuiltIn::SymbolTextAndIconShader,
                  shaders::BuiltIn::WideVectorShader>(shaders, programParameters);
}

} // namespace vulkan
} // namespace mbgl
