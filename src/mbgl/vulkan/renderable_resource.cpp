#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/constants.hpp>

namespace mbgl {
namespace vulkan {

SurfaceRenderableResource::~SurfaceRenderableResource() {
    backend.getDevice()->waitIdle(backend.getDispatcher());

    // specific order
    swapchainFramebuffers.clear();
    renderPass.reset();
    swapchainImageViews.clear();
    swapchainImages.clear();
    swapchain.reset();
    surface.reset();

    depthAllocation.reset();
    colorAllocations.clear();
}

void SurfaceRenderableResource::initColor(uint32_t w, uint32_t h) {
    const uint32_t imageCount = backend.getMaxFrames();

    colorAllocations.reserve(imageCount);
    swapchainImages.reserve(imageCount);

    colorFormat = vk::Format::eR8G8B8A8Unorm;
    extent = vk::Extent2D(w, h);

    const auto imageUsage = vk::ImageUsageFlags() | vk::ImageUsageFlagBits::eColorAttachment |
                            vk::ImageUsageFlagBits::eTransferSrc;

    const auto imageCreateInfo = vk::ImageCreateInfo()
                                     .setImageType(vk::ImageType::e2D)
                                     .setFormat(colorFormat)
                                     .setExtent({w, h, 1})
                                     .setMipLevels(1)
                                     .setArrayLayers(1)
                                     .setSamples(vk::SampleCountFlagBits::e1)
                                     .setTiling(vk::ImageTiling::eOptimal)
                                     .setUsage(imageUsage)
                                     .setSharingMode(vk::SharingMode::eExclusive)
                                     .setInitialLayout(vk::ImageLayout::eUndefined);

    VmaAllocationCreateInfo allocCreateInfo = {};

    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    for (uint32_t i = 0; i < imageCount; ++i) {
        colorAllocations.push_back(std::make_unique<ImageAllocation>(backend.getAllocator()));
        auto& colorAllocation = colorAllocations.back();

        if (!colorAllocation->create(allocCreateInfo, imageCreateInfo)) {
            mbgl::Log::Error(mbgl::Event::Render, "Vulkan color texture allocation failed");
        }

        swapchainImages.push_back(colorAllocation->image);
    }
}

void SurfaceRenderableResource::initSwapchain(uint32_t w, uint32_t h) {
    const auto& physicalDevice = backend.getPhysicalDevice();
    const auto& device = backend.getDevice();
    const auto& dispatcher = backend.getDispatcher();

    const std::vector<vk::SurfaceFormatKHR>& formats = physicalDevice.getSurfaceFormatsKHR(surface.get(), dispatcher);
    const auto& formatIt = std::find_if(formats.begin(), formats.end(), [](const vk::SurfaceFormatKHR& format) {
        return (format.format == vk::Format::eB8G8R8A8Unorm || format.format == vk::Format::eR8G8B8A8Unorm) &&
               format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    });

    if (formatIt == formats.end()) throw std::runtime_error("No suitable swapchain format found");

    // only vk::PresentModeKHR::eFifo (vsync on) is guaranteed
    if (presentMode != vk::PresentModeKHR::eFifo) {
        const auto& presentModes = physicalDevice.getSurfacePresentModesKHR(surface.get(), dispatcher);

        if (std::find(presentModes.begin(), presentModes.end(), presentMode) == presentModes.end()) {


            presentMode = vk::PresentModeKHR::eFifo;
        }
    }

    // pick surface size
    capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface.get(), dispatcher);

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        extent = capabilities.currentExtent;
    } else {
        // update values based on surface limits
        extent.width = std::min(std::max(w, capabilities.minImageExtent.width), capabilities.maxImageExtent.width);
        extent.height = std::min(std::max(h, capabilities.minImageExtent.height), capabilities.maxImageExtent.height);
    }

    if (hasSurfaceTransformSupport()) {
        if (capabilities.currentTransform & vk::SurfaceTransformFlagBitsKHR::eRotate90 ||
            capabilities.currentTransform & vk::SurfaceTransformFlagBitsKHR::eRotate270) {
            std::swap(extent.width, extent.height);
        }
    }

    uint32_t swapchainImageCount = capabilities.minImageCount + 1;
    // check surface limits (0 is unlimited)
    if (capabilities.maxImageCount > 0) swapchainImageCount = std::min(swapchainImageCount, capabilities.maxImageCount);

    auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
                                   .setSurface(surface.get())
                                   .setMinImageCount(swapchainImageCount)
                                   .setImageFormat(formatIt->format)
                                   .setImageColorSpace(formatIt->colorSpace)
                                   .setPresentMode(presentMode)
                                   .setImageExtent(extent)
                                   .setImageArrayLayers(1)
                                   .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    int32_t graphicsQueueIndex = backend.getGraphicsQueueIndex();
    int32_t presentQueueIndex = backend.getPresentQueueIndex();

    if (graphicsQueueIndex != presentQueueIndex) {
        // TODO if this scenario is widespread and performance is a problem (shouldn't be the case on most hardware)
        // rework to vk::SharingMode::eExclusive + queue ownership
        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        const std::array<uint32_t, 2> queueIndices = {static_cast<uint32_t>(graphicsQueueIndex),
                                                      static_cast<uint32_t>(presentQueueIndex)};
        swapchainCreateInfo.setQueueFamilyIndices(queueIndices);
    } else {
        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    swapchainCreateInfo.setPreTransform(hasSurfaceTransformSupport() ? capabilities.currentTransform
                                                                     : vk::SurfaceTransformFlagBitsKHR::eIdentity);
    swapchainCreateInfo.setClipped(VK_TRUE);

    if (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) {
        swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eInherit);
    } else {
        swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    }

    // update this when recreating
    swapchainCreateInfo.setOldSwapchain(swapchain.get());

    swapchain = device->createSwapchainKHRUnique(swapchainCreateInfo, nullptr, dispatcher);
    swapchainImages = device->getSwapchainImagesKHR(swapchain.get(), dispatcher);

    colorFormat = swapchainCreateInfo.imageFormat;
    extent = swapchainCreateInfo.imageExtent;

    acquireSemaphores.reserve(swapchainImages.size());
    presentSemaphores.reserve(swapchainImages.size());
    for (uint32_t index = 0; index < swapchainImages.size(); ++index) {
        acquireSemaphores.emplace_back(device->createSemaphoreUnique({}, nullptr, dispatcher));
        presentSemaphores.emplace_back(device->createSemaphoreUnique({}, nullptr, dispatcher));

        const auto indexStr = std::to_string(index);
        backend.setDebugName(acquireSemaphores.back().get(), "PresentSemaphore_" + indexStr);
        backend.setDebugName(presentSemaphores.back().get(), "AcquireSemaphore_" + indexStr);
    }
}

void SurfaceRenderableResource::initDepthStencil() {
    const auto& physicalDevice = backend.getPhysicalDevice();
    const auto& device = backend.getDevice();
    const auto& dispatcher = backend.getDispatcher();

    // check for depth format support
    const std::vector<vk::Format> formats = {
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD16UnormS8Uint,
    };

    const auto& formatIt = std::find_if(formats.begin(), formats.end(), [&](const auto& format) {
        const auto& formatProps = physicalDevice.getFormatProperties(format, dispatcher);
        return formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment;
    });

    if (formatIt == formats.end()) {
        return;
    }

    depthFormat = *formatIt;

    const auto imageUsage = vk::ImageUsageFlags() | vk::ImageUsageFlagBits::eDepthStencilAttachment |
                            vk::ImageUsageFlagBits::eTransientAttachment;

    const auto imageCreateInfo = vk::ImageCreateInfo()
                                     .setImageType(vk::ImageType::e2D)
                                     .setFormat(depthFormat)
                                     .setExtent({extent.width, extent.height, 1})
                                     .setMipLevels(1)
                                     .setArrayLayers(1)
                                     .setSamples(vk::SampleCountFlagBits::e1)
                                     .setTiling(vk::ImageTiling::eOptimal)
                                     .setUsage(imageUsage)
                                     .setSharingMode(vk::SharingMode::eExclusive)
                                     .setInitialLayout(vk::ImageLayout::eUndefined);

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    uint32_t lazyMemoryIndex = 0;
    uint32_t memoryTypeBits = std::numeric_limits<uint32_t>::max();
    if (vmaFindMemoryTypeIndex(backend.getAllocator(), memoryTypeBits, &allocCreateInfo, &lazyMemoryIndex) !=
        VK_SUCCESS) {
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }

    depthAllocation = std::make_unique<ImageAllocation>(backend.getAllocator());
    if (!depthAllocation->create(allocCreateInfo, imageCreateInfo)) {
        return;
    }

    const auto imageViewCreateInfo =
        vk::ImageViewCreateInfo()
            .setImage(depthAllocation->image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(depthFormat)
            .setComponents(vk::ComponentMapping()) // defaults to vk::ComponentSwizzle::eIdentity
            .setSubresourceRange(vk::ImageSubresourceRange(
                vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1));

    depthAllocation->imageView = device->createImageViewUnique(imageViewCreateInfo, nullptr, dispatcher);

    backend.setDebugName(depthAllocation->image, "SwapchainDepthImage");
    backend.setDebugName(depthAllocation->imageView.get(), "SwapchainDepthImageView");
}

void SurfaceRenderableResource::swap() {
    auto& context = static_cast<Context&>(backend.getContext());
    context.submitFrame();
}

const vk::Image SurfaceRenderableResource::getAcquiredImage() const {
    if (surface) {
        return swapchainImages[acquiredImageIndex];
    }

    return colorAllocations[acquiredImageIndex]->image;
}

const vk::Semaphore& SurfaceRenderableResource::getAcquireSemaphore() const {
    const auto& context = static_cast<const Context&>(backend.getContext());
    return acquireSemaphores[context.getCurrentFrameResourceIndex()].get();
}

const vk::Semaphore& SurfaceRenderableResource::getPresentSemaphore() const {
    return presentSemaphores[acquiredImageIndex].get();
}

bool SurfaceRenderableResource::hasSurfaceTransformSupport() const {
#ifdef __ANDROID__
    return surface && capabilities.supportedTransforms != vk::SurfaceTransformFlagBitsKHR::eIdentity;
#else
    return false;
#endif
}

bool SurfaceRenderableResource::didSurfaceTransformUpdate() const {
    const auto& physicalDevice = backend.getPhysicalDevice();
    const auto& updatedCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface.get(), backend.getDispatcher());

    return capabilities.currentTransform != updatedCapabilities.currentTransform;
}

float SurfaceRenderableResource::getRotation() {
    switch (capabilities.currentTransform) {
        default:
        case vk::SurfaceTransformFlagBitsKHR::eIdentity:
            return 0.0f * M_PI / 180.0f;

        case vk::SurfaceTransformFlagBitsKHR::eRotate90:
        case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90:
            return 90.0f * M_PI / 180.0f;

        case vk::SurfaceTransformFlagBitsKHR::eRotate180:
        case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate180:
            return 180.0f * M_PI / 180.0f;

        case vk::SurfaceTransformFlagBitsKHR::eRotate270:
        case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270:
            return 270.0f * M_PI / 180.0f;
    }
}

void SurfaceRenderableResource::init(uint32_t w, uint32_t h) {
    if (surface) {
        initSwapchain(w, h);
    } else {
        initColor(w, h);
    }

    const auto& device = backend.getDevice();
    const auto& dispatcher = backend.getDispatcher();

    // create swapchain image views
    swapchainImageViews.reserve(swapchainImages.size());

    auto imageViewCreateInfo = vk::ImageViewCreateInfo()
                                   .setViewType(vk::ImageViewType::e2D)
                                   .setFormat(colorFormat)
                                   .setComponents(vk::ComponentMapping()) // defaults to vk::ComponentSwizzle::eIdentity
                                   .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

    for (const auto& image : swapchainImages) {
        imageViewCreateInfo.setImage(image);
        swapchainImageViews.push_back(device->createImageViewUnique(imageViewCreateInfo, nullptr, dispatcher));

        const size_t index = swapchainImageViews.size() - 1;
        backend.setDebugName(image, "SwapchainImage_" + std::to_string(index));
        backend.setDebugName(image, "SwapchainImageView_" + std::to_string(index));
    }

    // depth resources
    initDepthStencil();

    // create render pass
    const auto colorLayout = surface ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eTransferSrcOptimal;

    const std::array<vk::AttachmentDescription, 2> attachments = {
        vk::AttachmentDescription()
            .setFormat(colorFormat)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(colorLayout),

        vk::AttachmentDescription()
            .setFormat(depthFormat)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setStencilLoadOp(vk::AttachmentLoadOp::eClear)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)};

    const vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
    const vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    const auto subpass = vk::SubpassDescription()
                             .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                             .setColorAttachmentCount(1)
                             .setColorAttachments(colorAttachmentRef)
                             .setPDepthStencilAttachment(&depthAttachmentRef);

    const std::array<vk::SubpassDependency, 2> dependencies = {
        vk::SubpassDependency()
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask({})
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite),

        vk::SubpassDependency()
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests |
                             vk::PipelineStageFlagBits::eLateFragmentTests)
            .setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests |
                             vk::PipelineStageFlagBits::eLateFragmentTests)
            .setSrcAccessMask({})
            .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite),
    };

    const auto renderPassCreateInfo =
        vk::RenderPassCreateInfo().setAttachments(attachments).setSubpasses(subpass).setDependencies(dependencies);

    renderPass = device->createRenderPassUnique(renderPassCreateInfo, nullptr, dispatcher);

    // create swapchain framebuffers
    swapchainFramebuffers.reserve(swapchainImageViews.size());

    auto framebufferCreateInfo = vk::FramebufferCreateInfo()
                                     .setRenderPass(renderPass.get())
                                     .setAttachmentCount(2)
                                     .setWidth(extent.width)
                                     .setHeight(extent.height)
                                     .setLayers(1);

    for (const auto& imageView : swapchainImageViews) {
        const std::array<vk::ImageView, 2> imageViews = {imageView.get(), depthAllocation->imageView.get()};

        framebufferCreateInfo.setAttachments(imageViews);
        swapchainFramebuffers.push_back(device->createFramebufferUnique(framebufferCreateInfo, nullptr, dispatcher));
    }
}

void SurfaceRenderableResource::recreateSwapchain() {
    if (!surface) return;

    backend.getDevice()->waitIdle(backend.getDispatcher());

    swapchainFramebuffers.clear();
    renderPass.reset();
    swapchainImageViews.clear();
    swapchainImages.clear();
    acquireSemaphores.clear();
    presentSemaphores.clear();

    init(extent.width, extent.height);
}

const vk::UniqueFramebuffer& SurfaceRenderableResource::getFramebuffer() const {
    return swapchainFramebuffers[acquiredImageIndex];
}

} // namespace vulkan
} // namespace mbgl
