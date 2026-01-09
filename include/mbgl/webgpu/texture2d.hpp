#pragma once

#include <mbgl/gfx/texture2d.hpp>
#include <webgpu/webgpu.h>
#include <mbgl/util/image.hpp>

#include <memory>

namespace mbgl {
namespace webgpu {

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
    Texture2D& setUsage(uint32_t usageFlags_) noexcept;
    void setSizeChangedCallback(std::function<void(const Size&)>) noexcept;
    WGPUTextureFormat getNativeFormat() const noexcept { return nativeFormat; }

    gfx::TexturePixelType getFormat() const noexcept override { return pixelFormat; }
    Size getSize() const noexcept override { return size; }
    size_t getDataSize() const noexcept override;
    size_t getPixelStride() const noexcept override;
    size_t numChannels() const noexcept override;

    void create() noexcept override;
    void upload(const void* pixelData, const Size& size) noexcept override;
    void upload() noexcept override;
    void uploadSubRegion(const void* pixelData, const Size& size, uint16_t xOffset, uint16_t yOffset) noexcept override;
    bool needsUpload() const noexcept override { return dirty; }

    WGPUTexture getTexture() const { return texture; }
    WGPUTextureView getTextureView() const { return textureView; }
    WGPUSampler getSampler() const { return sampler; }
    const SamplerState& getSamplerState() const noexcept { return samplerState; }

protected:
    Context& context;
    WGPUTexture texture = nullptr;
    WGPUTextureView textureView = nullptr;
    WGPUSampler sampler = nullptr;
    uint32_t usageFlags = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc;
    WGPUTextureFormat nativeFormat = WGPUTextureFormat_Undefined;

    Size size{0, 0};
    gfx::TexturePixelType pixelFormat = gfx::TexturePixelType::RGBA;
    gfx::TextureChannelDataType channelType = gfx::TextureChannelDataType::UnsignedByte;
    SamplerState samplerState;
    std::shared_ptr<PremultipliedImage> image;
    bool dirty = false;
    std::function<void(const Size&)> sizeChangedCallback;
};

} // namespace webgpu
} // namespace mbgl
#include <functional>
