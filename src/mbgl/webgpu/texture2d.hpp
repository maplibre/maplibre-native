#pragma once

#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/util/size.hpp>
#include <mbgl/util/premultiply.hpp>
#include <memory>

namespace mbgl {
namespace webgpu {

class Context;

class Texture2D : public gfx::Texture2D {
public:
    explicit Texture2D(Context& context);
    ~Texture2D() override;
    
    // Texture configuration
    Texture2D& setSamplerConfiguration(const gfx::Texture2D::SamplerState& samplerState) noexcept override;
    Texture2D& setFormat(gfx::TexturePixelType pixelFormat, gfx::TextureChannelDataType channelType) noexcept override;
    Texture2D& setSize(Size size_) noexcept override;
    Texture2D& setImage(std::shared_ptr<PremultipliedImage> image_) noexcept override;
    
    // Texture properties
    gfx::TexturePixelType getFormat() const noexcept override;
    Size getSize() const noexcept override;
    size_t getDataSize() const noexcept override;
    size_t getPixelStride() const noexcept override;
    size_t numChannels() const noexcept override;
    
    // Texture operations
    void create() noexcept override;
    void upload(const void* pixelData, const Size& size_) noexcept override;
    void uploadSubRegion(const void* pixelData, const Size& size, uint16_t xOffset, uint16_t yOffset) noexcept override;

private:
    Context& context;
    void* texture = nullptr;
    void* textureView = nullptr;
    void* sampler = nullptr;
    Size size_ = {256, 256};
    gfx::Texture2D::SamplerState samplerState;
    gfx::TexturePixelType pixelType = gfx::TexturePixelType::RGBA;
    gfx::TextureChannelDataType channelType = gfx::TextureChannelDataType::UnsignedByte;
    std::shared_ptr<PremultipliedImage> image;
};

} // namespace webgpu
} // namespace mbgl