#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/upload_pass.hpp>

#include <Metal/MTLDevice.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>
#include <Metal/MTLSampler.hpp>

namespace mbgl {
namespace mtl {

Texture2D::Texture2D(Context& context_)
    : context(context_) {}

Texture2D::~Texture2D() {}

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
    }
}

size_t Texture2D::numChannels() const noexcept {
    switch (pixelFormat) {
        case gfx::TexturePixelType::RGBA:
            return 4;
        case gfx::TexturePixelType::Alpha:
            return 1;
        default:
            return 0;
    }
}

MTL::PixelFormat Texture2D::getMetalPixelFormat() const noexcept {
    switch (channelType) {
        case gfx::TextureChannelDataType::UnsignedByte:
            switch (pixelFormat) {
                case gfx::TexturePixelType::RGBA:
                    return MTL::PixelFormat::PixelFormatRGBA8Unorm;
                case gfx::TexturePixelType::Alpha:
                    return MTL::PixelFormat::PixelFormatA8Unorm;
                default:
                    assert(false);
                    return MTL::PixelFormat::PixelFormatInvalid;
            }
        case gfx::TextureChannelDataType::HalfFloat:
            switch (pixelFormat) {
                case gfx::TexturePixelType::RGBA:
                    return MTL::PixelFormat::PixelFormatRGBA16Float;
                case gfx::TexturePixelType::Alpha:
                    return MTL::PixelFormat::PixelFormatR16Float;
                default:
                    assert(false);
                    return MTL::PixelFormat::PixelFormatInvalid;
            }
        default:
            assert(false);
            return MTL::PixelFormat::PixelFormatInvalid;
    }
}

void Texture2D::createMetalTexture() noexcept {
    if (size == Size{0, 0}) {
        return;
    }
    metalTexture.reset();

    const auto format = getMetalPixelFormat();
    if (format == MTL::PixelFormat::PixelFormatInvalid) {
        return;
    }

    // Create a new texture object
    if (auto textureDescriptor = NS::RetainPtr(MTL::TextureDescriptor::texture2DDescriptor(format, size.width, size.height, /*mipmapped=*/false))) {
        textureDescriptor->setUsage(usage);
        metalTexture = context.createMetalTexture(std::move(textureDescriptor));
    }

    if (metalTexture) {
        textureDirty = false;
        context.renderingStats().memTextures += getDataSize();
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

gfx::Texture2D& Texture2D::setUsage(MTL::TextureUsage usage_) noexcept {
    usage = usage_;
    textureDirty = true;
    return *this;
}

MTL::Texture* Texture2D::getMetalTexture() const noexcept {
    assert(metalTexture);
    return metalTexture.get();
}

void Texture2D::updateSamplerConfiguration() noexcept {
    auto samplerDescriptor = NS::RetainPtr(MTL::SamplerDescriptor::alloc()->init());
    samplerDescriptor->setMinFilter(samplerState.filter == gfx::TextureFilterType::Nearest
                                        ? MTL::SamplerMinMagFilterNearest
                                        : MTL::SamplerMinMagFilterLinear);
    samplerDescriptor->setMagFilter(samplerState.filter == gfx::TextureFilterType::Nearest
                                        ? MTL::SamplerMinMagFilterNearest
                                        : MTL::SamplerMinMagFilterLinear);
    samplerDescriptor->setSAddressMode(samplerState.wrapU == gfx::TextureWrapType::Clamp
                                           ? MTL::SamplerAddressModeClampToEdge
                                           : MTL::SamplerAddressModeRepeat);
    samplerDescriptor->setTAddressMode(samplerState.wrapV == gfx::TextureWrapType::Clamp
                                           ? MTL::SamplerAddressModeClampToEdge
                                           : MTL::SamplerAddressModeRepeat);
    metalSamplerState = context.createMetalSamplerState(samplerDescriptor);
}

void Texture2D::bind(const RenderPass& renderPass, int32_t location) noexcept {
    assert(!textureDirty);
    const auto& encoder = renderPass.getMetalEncoder();

    // Update the sampler state if it was changed after resource creation
    if (samplerStateDirty) {
        updateSamplerConfiguration();
    }

    encoder->setFragmentTexture(metalTexture.get(), location);
    encoder->setFragmentSamplerState(metalSamplerState.get(), location);
}

void Texture2D::unbind(const RenderPass& renderPass, int32_t location) noexcept {
    const auto& encoder = renderPass.getMetalEncoder();
    encoder->setFragmentTexture(nullptr, location);
    encoder->setFragmentSamplerState(nullptr, location);
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
    assert(metalTexture.get());
    assert(!textureDirty);

    MTL::Region region = MTL::Region::Make2D(xOffset, yOffset, size_.width, size_.height);
    NS::UInteger bytesPerRow = size_.width * getPixelStride();
    metalTexture->replaceRegion(region, 0, pixelData, bytesPerRow);
}

void Texture2D::upload() noexcept {
    if (image && image->valid()) {
        setFormat(gfx::TexturePixelType::RGBA, gfx::TextureChannelDataType::UnsignedByte);
        upload(image->data.get(), image->size);
        image.reset();
    }
}

} // namespace mtl
} // namespace mbgl
