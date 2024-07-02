#pragma once

#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>

#include <memory>

namespace mbgl {
namespace vulkan {

class Context;
class RenderPass;

class Texture2D : public gfx::Texture2D {
public:
    Texture2D(Context& context_);
    ~Texture2D() override;

    gfx::Texture2D& setSamplerConfiguration(const SamplerState&) noexcept override;
    gfx::Texture2D& setFormat(gfx::TexturePixelType, gfx::TextureChannelDataType) noexcept override;
    gfx::Texture2D& setSize(Size size_) noexcept override;
    gfx::Texture2D& setImage(std::shared_ptr<PremultipliedImage>) noexcept override;

    Size getSize() const noexcept override { return size; }
    size_t getDataSize() const noexcept override;
    size_t getPixelStride() const noexcept override;
    size_t numChannels() const noexcept override;

    void create() noexcept override;

    void upload(const void* pixelData, const Size& size_) noexcept override;
    void uploadSubRegion(const void* pixelData, const Size& size, uint16_t xOffset, uint16_t yOffset) noexcept override;
    void upload() noexcept override;

    bool needsUpload() const noexcept override { return !!imageData; };

    const vk::ImageLayout& getVulkanImageLayout() const { return imageLayout; }
    const vk::UniqueImageView& getVulkanImageView() const { return imageAllocation->imageView; }
    const vk::Sampler& getVulkanSampler();

private:
    static vk::Format vulkanFormat(const gfx::TexturePixelType, const gfx::TextureChannelDataType);
    static vk::Filter vulkanFilter(const gfx::TextureFilterType);
    static vk::SamplerAddressMode vulkanAddressMode(const gfx::TextureWrapType);

    void createTexture();
    void createSampler();

    void destroyTexture();
    void destroySampler();

    void transitionToTransferLayout(const vk::UniqueCommandBuffer&);
    void transitionToShaderReadLayout(const vk::UniqueCommandBuffer&);

private:
    Context& context;

    Size size{0, 0};
    gfx::TexturePixelType pixelFormat{gfx::TexturePixelType::RGBA};
    gfx::TextureChannelDataType channelType{gfx::TextureChannelDataType::UnsignedByte};
    SamplerState samplerState{};

    std::shared_ptr<PremultipliedImage> imageData{nullptr};
    bool textureDirty{true};
    bool samplerStateDirty{true};

    SharedImageAllocation imageAllocation;
    vk::ImageLayout imageLayout{vk::ImageLayout::eUndefined};

    vk::Sampler sampler{};
};

} // namespace vulkan
} // namespace mbgl
