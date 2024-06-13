#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/logging.hpp>

#include <cassert>
#include <string>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace mbgl {
namespace vulkan {

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_) {

}

RendererBackend::~RendererBackend() {

}

std::unique_ptr<gfx::Context> RendererBackend::createContext() {
    return std::make_unique<vulkan::Context>(*this);
}

std::vector<const char*> RendererBackend::getLayers() {
    return {
#ifndef NDEBUG
        "VK_LAYER_KHRONOS_validation"
#endif
    };
}

std::vector<const char*> RendererBackend::getInstanceExtensions() {
    return {
#ifndef NDEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };
}

std::vector<const char*> RendererBackend::getDeviceExtensions() {
    return getDefaultRenderable().getResource<RenderableResource>().getDeviceExtensions();
}

template <typename T, typename F>
bool checkAvailability(const std::vector<T>& availableValues,
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

        if (!found) 
            return false;
    }

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                    void* userData) {
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
    }

    mbgl::Log::Record(mbglSeverity, mbgl::Event::Render, callbackData->pMessage);

    return VK_FALSE;
}

void RendererBackend::createDebugCallback() {
    const vk::DebugUtilsMessageSeverityFlagsEXT severity = 
        vk::DebugUtilsMessageSeverityFlagsEXT() |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    const vk::DebugUtilsMessageTypeFlagsEXT type = 
        vk::DebugUtilsMessageTypeFlagsEXT() |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding;

    const auto& createInfo = vk::DebugUtilsMessengerCreateInfoEXT()
        .setMessageSeverity(severity)
        .setMessageType(type)
        .setPfnUserCallback(vkDebugCallback);

    debugCallback = instance->createDebugUtilsMessengerEXTUnique(createInfo);

    if (!debugCallback)
        mbgl::Log::Error(mbgl::Event::Render, "Failed to register Vulkan debug callback");
}

void RendererBackend::init() {
    initInstance();
    initSurface();
    initDevice();
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
    const auto& appInfo = vk::ApplicationInfo()
        .setPApplicationName("maplibre-natve")
        .setApplicationVersion(0)
        .setPEngineName("maplibre-natve")
        .setEngineVersion(0)
        .setApiVersion(VK_API_VERSION_1_1);

    vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(), &appInfo);

    const auto& layers = getLayers();

    bool layersAvailable = checkAvailability(vk::enumerateInstanceLayerProperties(), layers,
        [](const vk::LayerProperties& value) { return value.layerName.data(); });

    if (layersAvailable) {
        createInfo.setPEnabledLayerNames(layers);
    } else {
#ifndef NDEBUG
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan layers not found");
#endif
    }

    const auto& extensions = getInstanceExtensions();

    bool extensionsAvailable = checkAvailability(vk::enumerateInstanceExtensionProperties(), extensions, 
        [](const vk::ExtensionProperties& value) { return value.extensionName.data(); });

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

#ifndef NDEBUG
    // enable validation layer callback
    createDebugCallback();
#endif
}

void RendererBackend::initSurface() {
    getDefaultRenderable().getResource<RenderableResource>().createSurface();
}

void RendererBackend::initAllocator() {
    // TODO init VMA
}

void RendererBackend::initDevice() {

    const auto& extensions = getDeviceExtensions();
    const auto& layers = getLayers();
    const auto& surface = getDefaultRenderable().getResource<RenderableResource>().surface;

    const auto& isPhysicalDeviceCompatible = [&](const vk::PhysicalDevice& device) -> bool {
        bool extensionsAvailable = checkAvailability(
            device.enumerateDeviceExtensionProperties(), extensions, 
            [](const vk::ExtensionProperties& value) { return value.extensionName.data(); });

        if (!extensionsAvailable)
            return false;

        graphicsQueueIndex = -1;
        presentQueueIndex = -1;

        const auto& queues = device.getQueueFamilyProperties();

        for (auto i = 0u; i < queues.size(); ++i) {
            const auto& queue = queues[i];

            if (queue.queueCount == 0)
                continue;

            if (queue.queueFlags & vk::QueueFlagBits::eGraphics)
                graphicsQueueIndex = i;

            if (surface && device.getSurfaceSupportKHR(i, surface))
                presentQueueIndex = i;
            

            if (graphicsQueueIndex != -1 && (!surface || presentQueueIndex != -1))
                break;
        }

        if (graphicsQueueIndex == -1 || (surface && presentQueueIndex == -1))
            return false;

        if (surface) {
            if (device.getSurfaceFormatsKHR(surface).empty()) 
                return false;

            if (device.getSurfacePresentModesKHR(surface).empty()) 
                return false;
        }

        return true;
    };

    const auto& pickPhysicalDevice = [&]() {
        const auto& physicalDevices = instance->enumeratePhysicalDevices();
        if (physicalDevices.empty())
            throw std::runtime_error("No Vulkan compatible GPU found");
        
        for (const auto& device : physicalDevices) {
            if (isPhysicalDeviceCompatible(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (!physicalDevice) 
            throw std::runtime_error("No suitable GPU found");
    };

    pickPhysicalDevice();
    
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;

    queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags(), graphicsQueueIndex, 1, &queuePriority);

    if (surface && graphicsQueueIndex != presentQueueIndex)
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags(), presentQueueIndex, 1, &queuePriority);

    vk::PhysicalDeviceFeatures deviceFeatures;

    auto& createInfo = vk::DeviceCreateInfo()
        .setQueueCreateInfos(queueCreateInfos)
        .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
        .setPpEnabledExtensionNames(extensions.data())
        .setPEnabledFeatures(&deviceFeatures);

    // this is not needed for newer implementations
    createInfo.setPEnabledLayerNames(layers);

    device = physicalDevice.createDeviceUnique(createInfo);

    // optional function pointer specialization for device
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());

    graphicsQueue = device->getQueue(graphicsQueueIndex, 0);
    presentQueue = device->getQueue(presentQueueIndex, 0);
}

void RendererBackend::initSwapchain() {

    auto& renderableResource = getDefaultRenderable().getResource<RenderableResource>();
    const auto& surface = renderableResource.surface;
    if (!surface)
        return;

    const std::vector<vk::SurfaceFormatKHR>& formats = physicalDevice.getSurfaceFormatsKHR(surface);
    const auto& formatIt = std::find_if(formats.begin(), formats.end(), [](const vk::SurfaceFormatKHR& format) { 
        return format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    });

    if (formatIt == formats.end()) 
        throw std::runtime_error("No suitable swapchain format found");

    // only vk::PresentModeKHR::eFifo (vsync on) is guaranteed
    // TODO check for vk::PresentModeKHR::eImmediate when uncapped

    // pick surface size
    const vk::SurfaceCapabilitiesKHR& capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

    vk::Extent2D extent;
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        extent = capabilities.currentExtent;
    } else {
        // actual size should be set by the renderable constructor in this case
        const auto& renderableSize = getDefaultRenderable().getSize();

        // update values based on surface limits
        extent.width = std::min(std::max(renderableSize.width, capabilities.minImageExtent.width), capabilities.maxImageExtent.width);
        extent.height = std::min(std::max(renderableSize.height, capabilities.minImageExtent.height), capabilities.maxImageExtent.height);
    }

    uint32_t swapchainImageCount = capabilities.minImageCount + 1;
    // check surface limits (0 is unlimited)
    if (capabilities.maxImageCount > 0) 
        swapchainImageCount = std::min(swapchainImageCount, capabilities.maxImageCount);

    auto& swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
        .setSurface(surface)
        .setMinImageCount(swapchainImageCount)
        .setImageFormat(formatIt->format)
        .setImageColorSpace(formatIt->colorSpace)
        .setPresentMode(vk::PresentModeKHR::eFifo)
        .setImageExtent(extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    if (graphicsQueueIndex != presentQueueIndex) {
        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent); // revisit this
        const std::array<uint32_t, 2> queueIndices = { static_cast<uint32_t>(graphicsQueueIndex), static_cast<uint32_t>(presentQueueIndex) };
        swapchainCreateInfo.setQueueFamilyIndices(queueIndices);
    } else {
        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    swapchainCreateInfo.setPreTransform(capabilities.currentTransform);
    swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    swapchainCreateInfo.setClipped(VK_TRUE);

    // update this when recreating
    //swapchainCreateInfo.setOldSwapchain(vk::SwapchainKHR(nullptr));

    renderableResource.swapchain = device->createSwapchainKHRUnique(swapchainCreateInfo);
    renderableResource.swapchainImages = device->getSwapchainImagesKHR(renderableResource.swapchain.get());

    renderableResource.format = swapchainCreateInfo.imageFormat;
    renderableResource.extent = swapchainCreateInfo.imageExtent;

    // create swapchain image views
    renderableResource.swapchainImageViews.reserve(renderableResource.swapchainImages.size());

    auto& imageViewCreateInfo = vk::ImageViewCreateInfo()
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(renderableResource.format) 
        .setComponents(vk::ComponentMapping()) // defaults to vk::ComponentSwizzle::eIdentity
        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    for (const auto& image : renderableResource.swapchainImages) {
        imageViewCreateInfo.setImage(image);
        renderableResource.swapchainImageViews.push_back(device->createImageViewUnique(imageViewCreateInfo));
    }

    // create render pass
    // TODO refactor this
    const auto& colorAttachment = vk::AttachmentDescription(vk::AttachmentDescriptionFlags())
        .setFormat(renderableResource.format)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    const vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

    const auto& subpass = vk::SubpassDescription(vk::SubpassDescriptionFlags()) 
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&colorAttachmentRef);

     const auto& subpassDependency = vk::SubpassDependency()
        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead);

    const auto& renderPassCreateInfo = vk::RenderPassCreateInfo()
        .setAttachmentCount(1)
        .setPAttachments(&colorAttachment)
        .setSubpassCount(1)
        .setPSubpasses(&subpass)
        .setDependencyCount(1)
        .setPDependencies(&subpassDependency);

    renderableResource.renderPass = device->createRenderPassUnique(renderPassCreateInfo);

    // create swapchain framebuffers
    renderableResource.swapchainFramebuffers.reserve(renderableResource.swapchainImageViews.size());

    auto& framebufferCreateInfo = vk::FramebufferCreateInfo()
        .setRenderPass(renderableResource.renderPass.get())
        .setAttachmentCount(1)
        .setWidth(renderableResource.extent.width)
        .setHeight(renderableResource.extent.height)
        .setLayers(1);

    for (const auto& imageView : renderableResource.swapchainImageViews) {
        framebufferCreateInfo.setPAttachments(&imageView.get());
        renderableResource.swapchainFramebuffers.push_back(device->createFramebufferUnique(framebufferCreateInfo));
    }

    maxFrames = static_cast<uint32_t>(renderableResource.swapchainFramebuffers.size());
}

void RendererBackend::initCommandPool() {
    const vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsQueueIndex);
    commandPool = device->createCommandPoolUnique(createInfo);
}

/// @brief Register a list of types with a shader registry instance
/// @tparam ...ShaderID Pack of BuiltIn:: shader IDs
/// @param registry A shader registry instance
/// @param programParameters ProgramParameters used to initialize each instance
template <shaders::BuiltIn... ShaderID>
void registerTypes(gfx::ShaderRegistry& registry, const ProgramParameters& programParameters) {
    /// The following fold expression will create a shader for every type
    /// in the parameter pack and register it with the shader registry.

    /// Registration calls are wrapped in a lambda that throws on registration
    /// failure, we shouldn't expect registration to faill unless the shader
    /// registry instance provided already has conflicting programs present.
    //(
    //    [&]() {
    //        using namespace std::string_literals;
    //        using ShaderClass = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Vulkan>;
    //        auto group = std::make_shared<ShaderGroup<ShaderID>>(programParameters);
    //        if (!registry.registerShaderGroup(std::move(group), ShaderClass::name)) {
    //            assert(!"duplicate shader group");
    //            throw std::runtime_error("Failed to register "s + ShaderClass::name + " with shader registry!");
    //        }
    //    }(),
    //    ...);
}

void RendererBackend::initShaders(gfx::ShaderRegistry& shaders, const ProgramParameters& programParameters) {
    registerTypes<//shaders::BuiltIn::BackgroundShader,
                  //shaders::BuiltIn::BackgroundPatternShader,
                  shaders::BuiltIn::CircleShader//,
                  //shaders::BuiltIn::ClippingMaskProgram,
                  //shaders::BuiltIn::CollisionBoxShader,
                  //shaders::BuiltIn::CollisionCircleShader,
                  //shaders::BuiltIn::CustomSymbolIconShader,
                  //shaders::BuiltIn::DebugShader,
                  //shaders::BuiltIn::FillShader,
                  //shaders::BuiltIn::FillOutlineShader,
                  //shaders::BuiltIn::FillPatternShader,
                  //shaders::BuiltIn::FillOutlinePatternShader,
                  //shaders::BuiltIn::FillOutlineTriangulatedShader,
                  //shaders::BuiltIn::FillExtrusionShader,
                  //shaders::BuiltIn::FillExtrusionPatternShader,
                  //shaders::BuiltIn::HeatmapShader,
                  //shaders::BuiltIn::HeatmapTextureShader,
                  //shaders::BuiltIn::HillshadeShader,
                  //shaders::BuiltIn::HillshadePrepareShader,
                  //shaders::BuiltIn::LineShader,
                  //shaders::BuiltIn::LineGradientShader,
                  //shaders::BuiltIn::LineSDFShader,
                  //shaders::BuiltIn::LinePatternShader,
                  //shaders::BuiltIn::RasterShader,
                  //shaders::BuiltIn::SymbolIconShader,
                  //shaders::BuiltIn::SymbolSDFIconShader,
                  //shaders::BuiltIn::SymbolTextAndIconShader,
                  //shaders::BuiltIn::WideVectorShader
    >(shaders, programParameters);
}

} // namespace vulkan
} // namespace mbgl
