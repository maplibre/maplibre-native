#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/upload_pass.hpp>

namespace mbgl {
namespace vulkan {

Texture2D::Texture2D(Context& context_)
    : context(context_) {}

Texture2D::~Texture2D() {
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
    image = std::move(image_);
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

void Texture2D::createMetalTexture() noexcept {
    if (size == Size{0, 0}) {
        return;
    }
   
}

void Texture2D::create() noexcept {
    if (textureDirty) {
        createMetalTexture();
    }
    if (samplerStateDirty) {
        updateSamplerConfiguration();
    }
}

void Texture2D::updateSamplerConfiguration() noexcept {
    
}

void Texture2D::bind(RenderPass& renderPass, int32_t location) noexcept {
    assert(!textureDirty);

    // Update the sampler state if it was changed after resource creation
    if (samplerStateDirty) {
        updateSamplerConfiguration();
    }

    context.renderingStats().numTextureBindings++;
}

void Texture2D::unbind(RenderPass&, int32_t /*location*/) noexcept {
    context.renderingStats().numTextureBindings--;
}

void Texture2D::upload(const void* pixelData, const Size& size_) noexcept {
    setSize(size_);
    if (textureDirty) {
        createMetalTexture();
    }
    if (samplerStateDirty) {
        updateSamplerConfiguration();
    }
    if (pixelData) {
        uploadSubRegion(pixelData, size, 0, 0);
    }
}

void Texture2D::uploadSubRegion(const void* pixelData, const Size& size_, uint16_t xOffset, uint16_t yOffset) noexcept {
  
}

void Texture2D::upload() noexcept {
    if (image && image->valid()) {
        setFormat(gfx::TexturePixelType::RGBA, gfx::TextureChannelDataType::UnsignedByte);
        upload(image->data.get(), image->size);
        image.reset();
    }
}

} // namespace vulkan
} // namespace mbgl
