#include <mbgl/vulkan/dynamic_texture.hpp>
#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace vulkan {

#if DYNAMIC_TEXTURE_VULKAN_MULTITHREADED_UPLOAD

thread_local std::optional<vk::UniqueCommandPool> commandPool;

DynamicTexture::DynamicTexture(Context& context_, Size size, gfx::TexturePixelType pixelType)
    : gfx::DynamicTexture(context_, size, pixelType),
      context(context_) {
    texture->create();
    const vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                               context.getBackend().getGraphicsQueueIndex());
    commandPool = context.getBackend().getDevice()->createCommandPoolUnique(
        createInfo, nullptr, context.getBackend().getDispatcher());
}

void DynamicTexture::uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) {
    std::scoped_lock lock(mutex);
    const auto& rect = texHandle.getRectangle();
    const auto imageSize = Size(rect.w, rect.h);

    gfx::Texture2DPtr textureToBlit = context.createTexture2D();
    texturesToBlit.emplace(texHandle, textureToBlit);

    const auto& textureToBlitVK = static_cast<Texture2D*>(textureToBlit.get());
    textureToBlitVK->setSize(imageSize);
    textureToBlitVK->setFormat(texture.get()->getFormat(), gfx::TextureChannelDataType::UnsignedByte);
    textureToBlitVK->setUsage(Texture2DUsage::Blit);
    textureToBlitVK->create();

    std::vector<std::function<void(gfx::Context&)>> deletionQueue;
    context.submitOneTimeCommand(commandPool, [&](const vk::UniqueCommandBuffer& commandBuffer) {
        textureToBlitVK->uploadSubRegion(pixelData, imageSize, 0, 0, commandBuffer, &deletionQueue);
    });
    for (const auto& function : deletionQueue) function(context);
    deletionQueue.clear();

    gfx::DynamicTexture::uploadImage(pixelData, texHandle);
}

void DynamicTexture::uploadDeferredImages() {
    std::scoped_lock lock(mutex);

    const auto& textureVK = static_cast<Texture2D*>(texture.get());
    context.submitOneTimeCommand([&](const vk::UniqueCommandBuffer& commandBuffer) {
        textureVK->transitionToTransferWriteLayout(commandBuffer);
        for (const auto& pair : texturesToBlit) {
            const auto& rect = pair.first.getRectangle();
            const auto copyInfo = vk::ImageCopy()
                                      .setSrcSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
                                      .setDstSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
                                      .setExtent({rect.w, rect.h, 1})
                                      .setDstOffset({rect.x, rect.y, 0});

            const auto& textureToBlitVK = static_cast<Texture2D*>(pair.second.get());
            commandBuffer->copyImage(textureToBlitVK->getVulkanImage(),
                                     textureToBlitVK->getVulkanImageLayout(),
                                     textureVK->getVulkanImage(),
                                     textureVK->getVulkanImageLayout(),
                                     copyInfo,
                                     context.getBackend().getDispatcher());
        }
        textureVK->transitionToShaderReadLayout(commandBuffer);
    });
    texturesToBlit.clear();
}

bool DynamicTexture::removeTexture(const gfx::TextureHandle& texHandle) {
    if (gfx::DynamicTexture::removeTexture(texHandle)) {
        std::scoped_lock lock(mutex);
        texturesToBlit.erase(texHandle);
        return true;
    }
    return false;
}
#else

DynamicTexture::DynamicTexture(Context& context_, Size size, gfx::TexturePixelType pixelType)
    : gfx::DynamicTexture(context_, size, pixelType),
      context(context_) {
    texture->create();
}

void DynamicTexture::uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) {
    std::scoped_lock lock(mutex);
    const auto& rect = texHandle.getRectangle();

    const auto& backend = context.getBackend();
    const auto& allocator = backend.getAllocator();

    const auto bufferInfo = vk::BufferCreateInfo()
                                .setSize(static_cast<vk::DeviceSize>(rect.w) * rect.h * texture->getPixelStride())
                                .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
                                .setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationInfo = {};

    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    UniqueBufferAllocation bufferAllocation = std::make_unique<BufferAllocation>(allocator);
    if (!bufferAllocation->create(allocationInfo, bufferInfo)) {
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan texture buffer allocation failed");
        return;
    }

    vmaMapMemory(allocator, bufferAllocation->allocation, &bufferAllocation->mappedBuffer);
    memcpy(bufferAllocation->mappedBuffer, pixelData, bufferInfo.size);

    textureBuffersToUpload.emplace(texHandle, std::move(bufferAllocation));

    gfx::DynamicTexture::uploadImage(pixelData, texHandle);
}

void DynamicTexture::uploadDeferredImages() {
    std::scoped_lock lock(mutex);

    const auto& textureVK = static_cast<Texture2D*>(texture.get());
    context.submitOneTimeCommand([&](const vk::UniqueCommandBuffer& commandBuffer) {
        textureVK->transitionToTransferWriteLayout(commandBuffer);
        for (const auto& pair : textureBuffersToUpload) {
            const auto& rect = pair.first.getRectangle();
            const auto region = vk::BufferImageCopy()
                                    .setBufferOffset(0)
                                    .setBufferRowLength(rect.w)
                                    .setImageSubresource(
                                        vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
                                    .setImageOffset(vk::Offset3D(rect.x, rect.y))
                                    .setImageExtent(vk::Extent3D(rect.w, rect.h, 1));

            commandBuffer->copyBufferToImage(pair.second->buffer,
                                             textureVK->getVulkanImage(),
                                             textureVK->getVulkanImageLayout(),
                                             region,
                                             context.getBackend().getDispatcher());

            context.renderingStats().numTextureUpdates++;
            context.renderingStats().textureUpdateBytes += rect.w * rect.h * texture->getPixelStride();
        }
        textureVK->transitionToShaderReadLayout(commandBuffer);
    });
    textureBuffersToUpload.clear();
}

bool DynamicTexture::removeTexture(const gfx::TextureHandle& texHandle) {
    if (gfx::DynamicTexture::removeTexture(texHandle)) {
        std::scoped_lock lock(mutex);
        textureBuffersToUpload.erase(texHandle);
        return true;
    }
    return false;
}
#endif

} // namespace vulkan
} // namespace mbgl
