#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace vulkan {

static bool hasMemoryType(const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlagBits& type) {
    const auto& memoryProps = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memoryProps.memoryTypeCount; ++i) {
        if (memoryProps.memoryTypes[i].propertyFlags & type) {
            return true;
        }
    }

    return false;
}

SurfaceRenderableResource::~SurfaceRenderableResource() {
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

void SurfaceRenderableResource::initSwapchain(uint32_t w, uint32_t h, vk::PresentModeKHR presentMode) {
    const auto& physicalDevice = backend.getPhysicalDevice();
    const auto& device = backend.getDevice();

    const std::vector<vk::SurfaceFormatKHR>& formats = physicalDevice.getSurfaceFormatsKHR(surface.get());
    const auto& formatIt = std::find_if(formats.begin(), formats.end(), [](const vk::SurfaceFormatKHR& format) {
        return (format.format == vk::Format::eB8G8R8A8Unorm || format.format == vk::Format::eR8G8B8A8Unorm) &&
               format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    });

    if (formatIt == formats.end()) throw std::runtime_error("No suitable swapchain format found");

    // only vk::PresentModeKHR::eFifo (vsync on) is guaranteed
    if (presentMode != vk::PresentModeKHR::eFifo) {
        const std::vector<vk::PresentModeKHR>& presentModes = physicalDevice.getSurfacePresentModesKHR(surface.get());
        if (std::find(presentModes.begin(), presentModes.end(), presentMode) == presentModes.end()) {
            mbgl::Log::Error(
                mbgl::Event::Render,
                "Requested PresentModeKHR not available (" + std::to_string(static_cast<int>(presentMode)) + ")");

            presentMode = vk::PresentModeKHR::eFifo;
        }
    }

    // pick surface size
    const vk::SurfaceCapabilitiesKHR& capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface.get());

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        extent = capabilities.currentExtent;
    } else {
        // update values based on surface limits
        extent.width = std::min(std::max(w, capabilities.minImageExtent.width), capabilities.maxImageExtent.width);
        extent.height = std::min(std::max(h, capabilities.minImageExtent.height), capabilities.maxImageExtent.height);
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

    swapchainCreateInfo.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
    swapchainCreateInfo.setClipped(VK_TRUE);

    if (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) {
        swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eInherit);
    } else {
        swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    }

    // update this when recreating
    swapchainCreateInfo.setOldSwapchain(vk::SwapchainKHR(swapchain.get()));

    swapchain = device->createSwapchainKHRUnique(swapchainCreateInfo);
    swapchainImages = device->getSwapchainImagesKHR(swapchain.get());

    colorFormat = swapchainCreateInfo.imageFormat;
    extent = swapchainCreateInfo.imageExtent;
}

void SurfaceRenderableResource::initDepthStencil() {
    const auto& physicalDevice = backend.getPhysicalDevice();
    const auto& device = backend.getDevice();

    // check for depth format support
    const std::vector<vk::Format> formats = {
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD16UnormS8Uint,
    };

    const auto& formatIt = std::find_if(formats.begin(), formats.end(), [&](const auto& format) {
        const auto& formatProps = physicalDevice.getFormatProperties(format);
        return formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment;
    });

    if (formatIt == formats.end()) {
        mbgl::Log::Error(mbgl::Event::Render, "Depth/Stencil format not available");
        return;
    }

    depthFormat = *formatIt;

    const bool hasLazyMemory = hasMemoryType(physicalDevice, vk::MemoryPropertyFlagBits::eLazilyAllocated);
    const auto memoryUsage = hasLazyMemory ? VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED
                                           : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

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
    allocCreateInfo.usage = memoryUsage;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    depthAllocation = std::make_unique<ImageAllocation>(backend.getAllocator());
    if (!depthAllocation->create(allocCreateInfo, imageCreateInfo)) {
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan depth texture allocation failed");
        return;
    }

    const auto imageViewCreateInfo =
        vk::ImageViewCreateInfo()
            .setImage(depthAllocation->image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(depthFormat)
            .setComponents(vk::ComponentMapping()) // defaults to vk::ComponentSwizzle::eIdentity
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));

    depthAllocation->imageView = device->createImageViewUnique(imageViewCreateInfo);

    backend.setDebugName(vk::Image(depthAllocation->image), "SwapchainDepthImage");
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

void SurfaceRenderableResource::init(uint32_t w, uint32_t h) {
    if (surface) {
        initSwapchain(w, h);
    } else {
        initColor(w, h);
    }

    const auto& device = backend.getDevice();

    // create swapchain image views
    swapchainImageViews.reserve(swapchainImages.size());

    auto imageViewCreateInfo = vk::ImageViewCreateInfo()
                                   .setViewType(vk::ImageViewType::e2D)
                                   .setFormat(colorFormat)
                                   .setComponents(vk::ComponentMapping()) // defaults to vk::ComponentSwizzle::eIdentity
                                   .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

    for (const auto& image : swapchainImages) {
        imageViewCreateInfo.setImage(image);
        swapchainImageViews.push_back(device->createImageViewUnique(imageViewCreateInfo));

        const size_t index = swapchainImageViews.size() - 1;
        backend.setDebugName(vk::Image(image), "SwapchainImage_" + std::to_string(index));
        backend.setDebugName(vk::Image(image), "SwapchainImageView_" + std::to_string(index));
    }

    // depth resources
    initDepthStencil();

    // create render pass
    const auto colorLayout = surface ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eTransferSrcOptimal;
    const auto colorAttachment = vk::AttachmentDescription(vk::AttachmentDescriptionFlags())
                                     .setFormat(colorFormat)
                                     .setSamples(vk::SampleCountFlagBits::e1)
                                     .setLoadOp(vk::AttachmentLoadOp::eClear)
                                     .setStoreOp(vk::AttachmentStoreOp::eStore)
                                     .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                                     .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                                     .setInitialLayout(vk::ImageLayout::eUndefined)
                                     .setFinalLayout(colorLayout);

    const vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

    const auto depthAttachment = vk::AttachmentDescription()
                                     .setFormat(depthFormat)
                                     .setSamples(vk::SampleCountFlagBits::e1)
                                     .setLoadOp(vk::AttachmentLoadOp::eClear)
                                     .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                                     .setStencilLoadOp(vk::AttachmentLoadOp::eClear)
                                     .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                                     .setInitialLayout(vk::ImageLayout::eUndefined)
                                     .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    const vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    const auto subpass = vk::SubpassDescription()
                             .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                             .setColorAttachmentCount(1)
                             .setColorAttachments(colorAttachmentRef)
                             .setPDepthStencilAttachment(&depthAttachmentRef);

    const auto subpassSrcStageMask = vk::PipelineStageFlags() | vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                     vk::PipelineStageFlagBits::eLateFragmentTests;

    const auto subpassDstStageMask = vk::PipelineStageFlags() | vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                     vk::PipelineStageFlagBits::eEarlyFragmentTests;

    const auto subpassSrcAccessMask = vk::AccessFlags() | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    const auto subpassDstAccessMask = vk::AccessFlags() | vk::AccessFlagBits::eColorAttachmentWrite |
                                      vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    const auto subpassDependency = vk::SubpassDependency()
                                       .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                                       .setDstSubpass(0)
                                       .setSrcStageMask(subpassSrcStageMask)
                                       .setDstStageMask(subpassDstStageMask)
                                       .setSrcAccessMask(subpassSrcAccessMask)
                                       .setDstAccessMask(subpassDstAccessMask);

    const std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    const auto renderPassCreateInfo =
        vk::RenderPassCreateInfo().setAttachments(attachments).setSubpasses(subpass).setDependencies(subpassDependency);

    renderPass = device->createRenderPassUnique(renderPassCreateInfo);

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
        swapchainFramebuffers.push_back(device->createFramebufferUnique(framebufferCreateInfo));
    }
}

void SurfaceRenderableResource::recreateSwapchain() {
    if (!surface) return;

    backend.getDevice()->waitIdle();

    swapchainFramebuffers.clear();
    renderPass.reset();
    swapchainImageViews.clear();
    swapchainImages.clear();

    init(extent.width, extent.height);
}

const vk::UniqueFramebuffer& SurfaceRenderableResource::getFramebuffer() const {
    return swapchainFramebuffers[acquiredImageIndex];
}

} // namespace vulkan
} // namespace mbgl
