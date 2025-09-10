#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/webgpu/context.hpp>

namespace mbgl {
namespace webgpu {

Texture2D::Texture2D(Context& context_)
    : context(context_) {
}

Texture2D::~Texture2D() {
    // TODO: Release WebGPU resources
}

gfx::Texture2D& Texture2D::setSamplerConfiguration(const gfx::Texture2D::SamplerState& samplerState_) noexcept {
    samplerState = samplerState_;
    // TODO: Update WebGPU sampler
    return *this;
}

gfx::Texture2D& Texture2D::setFormat(gfx::TexturePixelType pixelFormat_, gfx::TextureChannelDataType channelType_) noexcept {
    pixelFormat = pixelFormat_;
    channelType = channelType_;
    return *this;
}

gfx::Texture2D& Texture2D::setSize(Size size_) noexcept {
    size = size_;
    return *this;
}

gfx::Texture2D& Texture2D::setImage(std::shared_ptr<PremultipliedImage> image_) noexcept {
    image = image_;
    if (image) {
        size = image->size;
    }
    return *this;
}





size_t Texture2D::getDataSize() const noexcept {
    return size.width * size.height * getPixelStride();
}

size_t Texture2D::getPixelStride() const noexcept {
    switch (pixelFormat) {
        case gfx::TexturePixelType::Alpha:
            return 1;
        case gfx::TexturePixelType::Luminance:
            return 1;
        case gfx::TexturePixelType::RGBA:
            return 4;
        default:
            return 4;
    }
}

size_t Texture2D::numChannels() const noexcept {
    switch (pixelFormat) {
        case gfx::TexturePixelType::Alpha:
            return 1;
        case gfx::TexturePixelType::Luminance:
            return 1;
        case gfx::TexturePixelType::RGBA:
            return 4;
        default:
            return 4;
    }
}

void Texture2D::create() noexcept {
    // TODO: Create WebGPU texture
}

void Texture2D::upload(const void* pixelData, const Size& size_) noexcept {
    size = size_;
    // TODO: Upload data to WebGPU texture
}

void Texture2D::upload() noexcept {
    if (image) {
        upload(image->data.get(), image->size);
        dirty = false;
    }
}

void Texture2D::uploadSubRegion(const void* pixelData, const Size& regionSize, uint16_t xOffset, uint16_t yOffset) noexcept {
    // TODO: Upload subregion to WebGPU texture
}

} // namespace webgpu
} // namespace mbgl