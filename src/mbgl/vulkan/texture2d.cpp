#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/upload_pass.hpp>
#include <mbgl/vulkan/buffer_resource.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace vulkan {

bool ImageAllocation::create(const VmaAllocationCreateInfo& allocInfo, const vk::ImageCreateInfo& imageInfo) {
    VkImage image_;
    VkResult result = vmaCreateImage(
        allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &allocInfo, &image_, &allocation, nullptr);

    if (result != VK_SUCCESS) {
        return false;
    }

    image = vk::Image(image_);
    return true;
}

void ImageAllocation::destroy() {
    imageView.reset();
    vmaDestroyImage(allocator, VkImage(image), allocation);
    image = nullptr;
}

void ImageAllocation::setName([[maybe_unused]] const std::string& name) const {
#ifdef ENABLE_VMA_DEBUG
    if (allocation) {
        vmaSetAllocationName(allocator, allocation, name.data());
    }
#endif
}

Texture2D::Texture2D(Context& context_)
    : context(context_) {}

Texture2D::~Texture2D() {
    destroyTexture();
    destroySampler();

    context.renderingStats().numActiveTextures--;
    context.renderingStats().memTextures -= Texture2D::getDataSize();
}

gfx::Texture2D& Texture2D::setSamplerConfiguration(const SamplerState& samplerState_) noexcept {
    samplerState = samplerState_;
    samplerStateDirty = true;
    return *this;
}

gfx::Texture2D& Texture2D::setFormat(gfx::TexturePixelType pixelFormat_,
                                     gfx::TextureChannelDataType channelType_) noexcept {
    if (pixelFormat_ == pixelFormat && channelType_ == channelType) {
        return *this;
    }
    pixelFormat = pixelFormat_;
    channelType = channelType_;
    textureDirty = true;
    return *this;
}

gfx::Texture2D& Texture2D::setSize(mbgl::Size size_) noexcept {
    if (size_ == size) {
        return *this;
    }
    size = size_;
    textureDirty = true;
    return *this;
}

gfx::Texture2D& Texture2D::setImage(std::shared_ptr<PremultipliedImage> image_) noexcept {
    imageData = std::move(image_);
    return *this;
}

gfx::Texture2D& Texture2D::setUsage(Texture2DUsage value) noexcept {
    textureUsage = value;
    textureDirty = true;
    return *this;
}

size_t Texture2D::getDataSize() const noexcept {
    return Texture2D::getPixelStride() * size.width * size.height;
}

size_t Texture2D::getPixelStride() const noexcept {
    switch (channelType) {
        case gfx::TextureChannelDataType::UnsignedByte:
            return 1 * Texture2D::numChannels();
        case gfx::TextureChannelDataType::HalfFloat:
            return 2 * Texture2D::numChannels();
        case gfx::TextureChannelDataType::Float:
            return 4 * Texture2D::numChannels();
        default:
            return 0;
    }
}

size_t Texture2D::numChannels() const noexcept {
    switch (pixelFormat) {
        case gfx::TexturePixelType::RGBA:
            return 4;
        case gfx::TexturePixelType::Alpha:
            return 1;
        case gfx::TexturePixelType::Stencil:
            return 1;
        case gfx::TexturePixelType::Depth:
            return 1;
        default:
            return 0;
    }
}

void Texture2D::create() noexcept {
    if (textureDirty) {
        createTexture();
    }
}

void Texture2D::upload() noexcept {
    if (!imageData) return;

    upload(imageData->data.get(), imageData->size);

    imageData.reset();
}

void Texture2D::upload(const void* pixelData, const Size& size_) noexcept {
    setSize(size_);
    uploadSubRegion(pixelData, size_, 0, 0);
}

void Texture2D::uploadSubRegion(const void* pixelData, const Size& size_, uint16_t xOffset, uint16_t yOffset) noexcept {
    if (!pixelData || size_.width == 0 || size_.height == 0) return;

    const auto& encoder = context.createCommandEncoder();
    const auto& encoderImpl = static_cast<const CommandEncoder&>(*encoder);

    uploadSubRegion(pixelData, size_, xOffset, yOffset, encoderImpl.getCommandBuffer());
}

void Texture2D::uploadSubRegion(const void* pixelData,
                                const Size& size_,
                                uint16_t xOffset,
                                uint16_t yOffset,
                                const vk::UniqueCommandBuffer& commandBuffer) noexcept {
    if (!pixelData || size_.width == 0 || size_.height == 0) return;

    create();

    if (!imageAllocation) return;

    const auto& allocator = context.getBackend().getAllocator();

    const auto bufferInfo = vk::BufferCreateInfo()
                                .setSize(static_cast<vk::DeviceSize>(size_.width) * size_.height * getPixelStride())
                                .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
                                .setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationInfo = {};

    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    SharedBufferAllocation bufferAllocation = std::make_shared<BufferAllocation>(allocator);
    if (!bufferAllocation->create(allocationInfo, bufferInfo)) {
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan texture buffer allocation failed");
        return;
    }

    vmaMapMemory(allocator, bufferAllocation->allocation, &bufferAllocation->mappedBuffer);
    memcpy(bufferAllocation->mappedBuffer, pixelData, bufferInfo.size);

    const auto enqueueCommands = [&](const auto& buffer) {
        transitionToTransferLayout(buffer);

        const auto region = vk::BufferImageCopy()
                                .setBufferOffset(0)
                                .setBufferRowLength(size_.width)
                                .setImageSubresource(
                                    vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
                                .setImageOffset(vk::Offset3D(xOffset, yOffset))
                                .setImageExtent(vk::Extent3D(size_.width, size_.height, 1));

        buffer->copyBufferToImage(bufferAllocation->buffer, imageAllocation->image, imageLayout, region);

        transitionToShaderReadLayout(buffer);
    };

    enqueueCommands(commandBuffer);

    context.enqueueDeletion([buffAlloc = std::move(bufferAllocation)](const auto&) mutable { buffAlloc.reset(); });

    context.renderingStats().numTextureUpdates++;
}

vk::Format Texture2D::vulkanFormat(const gfx::TexturePixelType pixel, gfx::TextureChannelDataType channel) {
    // depth/stencil are packed formats
    if (pixel == gfx::TexturePixelType::Stencil) return vk::Format::eS8Uint;

    if (pixel == gfx::TexturePixelType::Depth) return vk::Format::eUndefined;

    if (pixel == gfx::TexturePixelType::Alpha) {
        switch (channel) {
            case gfx::TextureChannelDataType::UnsignedByte:
                return vk::Format::eR8Unorm;
            case gfx::TextureChannelDataType::HalfFloat:
                return vk::Format::eR16Sfloat;
            case gfx::TextureChannelDataType::Float:
                return vk::Format::eR32Sfloat;
        }
    }

    if (pixel == gfx::TexturePixelType::RGBA) {
        switch (channel) {
            case gfx::TextureChannelDataType::UnsignedByte:
                return vk::Format::eR8G8B8A8Unorm;
            case gfx::TextureChannelDataType::HalfFloat:
                return vk::Format::eR16G16B16A16Sfloat;
            case gfx::TextureChannelDataType::Float:
                return vk::Format::eR32G32B32A32Sfloat;
        }
    }

    return vk::Format::eUndefined;
}

vk::Filter Texture2D::vulkanFilter(const gfx::TextureFilterType type) {
    switch (type) {
        default:
            [[fallthrough]];
        case gfx::TextureFilterType::Nearest:
            return vk::Filter::eNearest;
        case gfx::TextureFilterType::Linear:
            return vk::Filter::eLinear;
    }
}

vk::SamplerAddressMode Texture2D::vulkanAddressMode(const gfx::TextureWrapType type) {
    switch (type) {
        default:
            [[fallthrough]];
        case gfx::TextureWrapType::Clamp:
            return vk::SamplerAddressMode::eClampToEdge;
        case gfx::TextureWrapType::Repeat:
            return vk::SamplerAddressMode::eRepeat;
    }
}

void Texture2D::createTexture() {
    if (size.width == 0 || size.height == 0) return;

    const auto& backend = context.getBackend();

    const auto format = vulkanFormat(pixelFormat, channelType);

    vk::ImageUsageFlags imageUsage;
    vk::ImageTiling imageTiling;

    switch (textureUsage) {
        default:
        case Texture2DUsage::ShaderInput:
            imageUsage = vk::ImageUsageFlags() | vk::ImageUsageFlagBits::eTransferDst |
                         vk::ImageUsageFlagBits::eSampled;
            imageTiling = vk::ImageTiling::eOptimal;
            break;

        case Texture2DUsage::Attachment:
            imageUsage = vk::ImageUsageFlags() | vk::ImageUsageFlagBits::eColorAttachment |
                         vk::ImageUsageFlagBits::eSampled;
            imageTiling = vk::ImageTiling::eOptimal;
            break;

        case Texture2DUsage::Read:
            imageUsage = vk::ImageUsageFlags() | vk::ImageUsageFlagBits::eTransferDst;
            imageTiling = vk::ImageTiling::eLinear;
            break;
    }

    const auto imageCreateInfo = vk::ImageCreateInfo()
                                     .setImageType(vk::ImageType::e2D)
                                     .setFormat(format)
                                     .setExtent({size.width, size.height, 1})
                                     .setMipLevels(1)
                                     .setArrayLayers(1)
                                     .setSamples(vk::SampleCountFlagBits::e1)
                                     .setTiling(imageTiling)
                                     .setUsage(imageUsage)
                                     .setSharingMode(vk::SharingMode::eExclusive)
                                     .setInitialLayout(vk::ImageLayout::eUndefined);

    VmaAllocationCreateInfo allocCreateInfo = {};

    if (textureUsage != Texture2DUsage::Read) {
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    } else {
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    }

    imageAllocation = std::make_shared<ImageAllocation>(backend.getAllocator());
    if (!imageAllocation->create(allocCreateInfo, imageCreateInfo)) {
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan texture allocation failed");
        return;
    }

    // defaults to eIdentity
    vk::ComponentMapping imageSwizzle;

    // alpha only format (eA8UnormKHR) is not part of core
    // use a R8 texture and map red channel to alpha
    if (pixelFormat == gfx::TexturePixelType::Alpha) {
        imageSwizzle = vk::ComponentMapping(vk::ComponentSwizzle::eZero,
                                            vk::ComponentSwizzle::eZero,
                                            vk::ComponentSwizzle::eZero,
                                            vk::ComponentSwizzle::eR);
    }

    if (textureUsage != Texture2DUsage::Read) {
        auto imageViewCreateInfo = vk::ImageViewCreateInfo()
                                       .setImage(imageAllocation->image)
                                       .setViewType(vk::ImageViewType::e2D)
                                       .setFormat(format)
                                       .setComponents(imageSwizzle)
                                       .setSubresourceRange(
                                           vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

        imageAllocation->imageView = backend.getDevice()->createImageViewUnique(imageViewCreateInfo);
    }

    // if the image is used as an attachment
    // it's layout is managed by the render pass
    if (textureUsage == Texture2DUsage::Attachment) {
        imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    } else {
        imageLayout = imageCreateInfo.initialLayout;
    }

    context.renderingStats().numCreatedTextures++;
    context.renderingStats().numActiveTextures++;
    context.renderingStats().memTextures += getDataSize();
    textureDirty = false;
}

void Texture2D::createSampler() {
    destroySampler();

    const auto filter = vulkanFilter(samplerState.filter);
    const auto addressModeU = vulkanAddressMode(samplerState.wrapU);
    const auto addressModeV = vulkanAddressMode(samplerState.wrapV);

    const auto samplerCreateInfo = vk::SamplerCreateInfo()
                                       .setMinFilter(filter)
                                       .setMagFilter(filter)
                                       .setMipmapMode(vk::SamplerMipmapMode::eNearest)
                                       .setMinLod(-VK_LOD_CLAMP_NONE)
                                       .setMaxLod(VK_LOD_CLAMP_NONE)
                                       .setAddressModeU(addressModeU)
                                       .setAddressModeV(addressModeV)
                                       .setAddressModeW(vk::SamplerAddressMode::eRepeat)
                                       .setAnisotropyEnable(false);

    sampler = context.getBackend().getDevice()->createSampler(samplerCreateInfo);

    samplerStateDirty = false;
}

void Texture2D::destroyTexture() {
    if (imageAllocation) {
        context.enqueueDeletion([allocation = std::move(imageAllocation)](const auto&) mutable { allocation.reset(); });

        imageLayout = vk::ImageLayout::eUndefined;
    }
}

void Texture2D::destroySampler() {
    if (sampler) {
        context.enqueueDeletion([sampler_ = std::move(sampler)](const auto& context_) mutable {
            context_.getBackend().getDevice()->destroySampler(sampler_);
        });

        sampler = nullptr;
    }
}

void Texture2D::transitionToTransferLayout(const vk::UniqueCommandBuffer& buffer) {
    const auto barrier = vk::ImageMemoryBarrier()
                             .setImage(imageAllocation->image)
                             .setOldLayout(imageLayout)
                             .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                             .setSrcAccessMask({})
                             .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                             .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setSubresourceRange(
                                 vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    buffer->pipelineBarrier(
        vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrier);

    imageLayout = barrier.newLayout;
}

void Texture2D::transitionToShaderReadLayout(const vk::UniqueCommandBuffer& buffer) {
    const auto barrier = vk::ImageMemoryBarrier()
                             .setImage(imageAllocation->image)
                             .setOldLayout(imageLayout)
                             .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                             .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                             .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                             .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setSubresourceRange(
                                 vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    buffer->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                            vk::PipelineStageFlagBits::eFragmentShader,
                            {},
                            nullptr,
                            nullptr,
                            barrier);

    imageLayout = barrier.newLayout;
}

void Texture2D::transitionToGeneralLayout(const vk::UniqueCommandBuffer& buffer) {
    const auto barrier = vk::ImageMemoryBarrier()
                             .setImage(imageAllocation->image)
                             .setOldLayout(imageLayout)
                             .setNewLayout(vk::ImageLayout::eGeneral)
                             .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                             .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                             .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setSubresourceRange(
                                 vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    buffer->pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrier);

    imageLayout = barrier.newLayout;
}

const vk::Sampler& Texture2D::getVulkanSampler() {
    if (samplerStateDirty) {
        createSampler();
    }

    return sampler;
}

void Texture2D::copyImage(vk::Image image) {
    if (!image) return;

    create();

    context.submitOneTimeCommand([&](const vk::UniqueCommandBuffer& commandBuffer) {
        const auto copyInfo = vk::ImageCopy()
                                  .setSrcSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
                                  .setDstSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
                                  .setExtent({size.width, size.height, 1});

        transitionToTransferLayout(commandBuffer);
        commandBuffer->copyImage(
            image, vk::ImageLayout::eTransferSrcOptimal, imageAllocation->image, imageLayout, copyInfo);
        transitionToGeneralLayout(commandBuffer);
    });
}

std::shared_ptr<PremultipliedImage> Texture2D::readImage() {
    if (!imageData) {
        imageData = std::make_shared<PremultipliedImage>();
    }

    // check for offset/padding
    const auto& device = context.getBackend().getDevice();
    const auto& layout = device->getImageSubresourceLayout(imageAllocation->image,
                                                           vk::ImageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0));

    imageData->resize(size);
    const auto& imageSize = getDataSize();

    void* mappedData_ = nullptr;
    vmaMapMemory(context.getBackend().getAllocator(), imageAllocation->allocation, &mappedData_);

    uint8_t* mappedData = reinterpret_cast<uint8_t*>(mappedData_) + layout.offset;

    if (imageSize == layout.arrayPitch) {
        memcpy(imageData->data.get(), mappedData, imageSize);
    } else {
        uint32_t rowSize = size.width * getPixelStride();
        for (uint32_t i = 0; i < size.height; ++i) {
            memcpy(imageData->data.get() + rowSize * i, mappedData + layout.rowPitch * i, rowSize);
        }
    }

    vmaUnmapMemory(context.getBackend().getAllocator(), imageAllocation->allocation);

    return imageData;
}

} // namespace vulkan
} // namespace mbgl
