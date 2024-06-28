#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/upload_pass.hpp>
#include <mbgl/vulkan/buffer_resource.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace vulkan {

Texture2D::Texture2D(Context& context_)
    : context(context_) {}

Texture2D::~Texture2D() {
    destroyTexture();
    destroySampler();

    context.renderingStats().numActiveTextures--;
    context.renderingStats().memTextures -= getDataSize();
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

size_t Texture2D::getDataSize() const noexcept {
    return size.width * size.height * getPixelStride();
}

size_t Texture2D::getPixelStride() const noexcept {
    switch (channelType) {
        case gfx::TextureChannelDataType::UnsignedByte:
            return 1 * numChannels();
        case gfx::TextureChannelDataType::HalfFloat:
            return 2 * numChannels();
        case gfx::TextureChannelDataType::Float:
            return 4 * numChannels();
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

    if (samplerStateDirty) {
        createSampler();
    }
}

void Texture2D::upload(const void* pixelData, const Size& size_) noexcept {
    setSize(size_);
    uploadSubRegion(pixelData, size_, 0, 0);
}

void Texture2D::uploadSubRegion(const void* pixelData, const Size& size_, uint16_t xOffset, uint16_t yOffset) noexcept {
    create();

    if (!imageAllocation) return;

    const auto& backend = context.getBackend();
    const auto& allocator = context.getBackend().getAllocator();

    const auto& bufferInfo = vk::BufferCreateInfo()
                                 .setSize(size_.width * size_.height * getPixelStride())
                                 .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
                                 .setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationInfo = {};

    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    SharedBufferAllocation bufferAllocation = std::make_shared<BufferAllocation>(allocator);

    VkResult result = vmaCreateBuffer(allocator,
                                      &VkBufferCreateInfo(bufferInfo),
                                      &allocationInfo,
                                      &bufferAllocation->buffer,
                                      &bufferAllocation->allocation,
                                      nullptr);
    if (result != VK_SUCCESS) {
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan texture buffer allocation failed");
        return;
    }

    vmaMapMemory(allocator, bufferAllocation->allocation, &bufferAllocation->mappedBuffer);
    memcpy(bufferAllocation->mappedBuffer, pixelData, bufferInfo.size);

    const auto enqueueCommands = [&](const auto& buffer) {
        transitionToTransferLayout(buffer);

        const auto& region = vk::BufferImageCopy()
                                 .setBufferOffset(0)
                                 .setBufferRowLength(size_.width)
                                 .setImageSubresource(
                                     vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
                                 .setImageOffset(vk::Offset3D(xOffset, yOffset))
                                 .setImageExtent(vk::Extent3D(size_.width, size_.height, 1));

        buffer->copyBufferToImage(bufferAllocation->buffer, imageAllocation->image, imageLayout, region);

        transitionToShaderReadLayout(buffer);
    };

    const auto& encoder = context.createCommandEncoder();
    const auto& encoderImpl = static_cast<const CommandEncoder&>(*encoder);

    enqueueCommands(encoderImpl.getCommandBuffer());

    context.enqueueDeletion(
        [buffAlloc = std::move(bufferAllocation)](const auto& context) mutable { buffAlloc.reset(); });

    context.renderingStats().numTextureUpdates++;
}

void Texture2D::upload() noexcept {
    if (!imageData) return;

    upload(imageData->data.get(), size);

    imageData.reset();
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
    const auto imageUsage = vk::ImageUsageFlags() | vk::ImageUsageFlagBits::eTransferDst |
                            vk::ImageUsageFlagBits::eSampled;

    const auto& imageCreateInfo = vk::ImageCreateInfo()
                                      .setImageType(vk::ImageType::e2D)
                                      .setFormat(format)
                                      .setExtent({size.width, size.height, 1})
                                      .setMipLevels(1)
                                      .setArrayLayers(1)
                                      .setSamples(vk::SampleCountFlagBits::e1)
                                      .setTiling(vk::ImageTiling::eOptimal)
                                      .setUsage(imageUsage)
                                      .setSharingMode(vk::SharingMode::eExclusive)
                                      .setInitialLayout(vk::ImageLayout::eUndefined);

    imageAllocation = std::make_shared<ImageAllocation>(backend.getAllocator());

    VmaAllocationCreateInfo allocCreateInfo = {};

    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocCreateInfo.flags = 0;

    VkResult result = vmaCreateImage(imageAllocation->allocator,
                                     &VkImageCreateInfo(imageCreateInfo),
                                     &allocCreateInfo,
                                     &imageAllocation->image,
                                     &imageAllocation->allocation,
                                     nullptr);

    if (result != VK_SUCCESS) {
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan texture allocation failed");
        return;
    }

    auto& imageViewCreateInfo = vk::ImageViewCreateInfo()
                                    .setImage(imageAllocation->image)
                                    .setViewType(vk::ImageViewType::e2D)
                                    .setFormat(format)
                                    .setComponents(vk::ComponentMapping()) // this can be changed for non-RGBA types
                                    .setSubresourceRange(
                                        vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    imageAllocation->imageView = backend.getDevice()->createImageViewUnique(imageViewCreateInfo);
    imageLayout = imageCreateInfo.initialLayout;

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

    const auto& samplerCreateInfo = vk::SamplerCreateInfo()
                                        .setMinFilter(filter)
                                        .setMagFilter(filter)
                                        .setAddressModeU(addressModeU)
                                        .setAddressModeV(addressModeV)
                                        .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
                                        .setAnisotropyEnable(false)
                                        .setCompareEnable(false);

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
        context.enqueueDeletion([sampler_ = std::move(sampler)](const auto& context) mutable {
            context.getBackend().getDevice()->destroySampler(sampler_);
        });

        sampler = nullptr;
    }
}

void Texture2D::transitionToTransferLayout(const vk::UniqueCommandBuffer& buffer) {
    const auto& barrier = vk::ImageMemoryBarrier()
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
    const auto& barrier = vk::ImageMemoryBarrier()
                              .setImage(imageAllocation->image)
                              .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
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

} // namespace vulkan
} // namespace mbgl
