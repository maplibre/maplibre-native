#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/upload_pass.hpp>

#include <Metal/MTLDevice.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>
#include <Metal/MTLSampler.hpp>
#include <Metal/MTLTexture.hpp>

namespace mbgl {
namespace mtl {

Texture2D::Texture2D(Context& context_)
    : context(context_) {}

Texture2D::~Texture2D() {}

gfx::Texture2D& Texture2D::setSamplerConfiguration(const SamplerState& samplerState_) noexcept {
    samplerState = samplerState_;
    samplerStateDirty = metalTexture.get() != nullptr;
    return *this;
}

gfx::Texture2D& Texture2D::setFormat(gfx::TexturePixelType pixelFormat_,
                                     gfx::TextureChannelDataType channelType_) noexcept {
    if (pixelFormat_ == pixelFormat && channelType_ == channelType) {
        return *this;
    }

    assert(!metalTexture.get());
    pixelFormat = pixelFormat_;
    channelType = channelType_;
    storageDirty = true;
    return *this;
}

gfx::Texture2D& Texture2D::setSize(mbgl::Size size_) noexcept {
    size = size_;
    storageDirty = true;
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
            }
        case gfx::TextureChannelDataType::HalfFloat:
            switch (pixelFormat) {
                case gfx::TexturePixelType::RGBA:
                    return MTL::PixelFormat::PixelFormatRGBA16Float;
                case gfx::TexturePixelType::Alpha:
                    return MTL::PixelFormat::PixelFormatR16Float;
                default:
                    assert(false);
            }
    }
}

void Texture2D::createObject() noexcept {
    // Create a new texture object
    assert(!metalTexture.get());
    auto textureDescriptor = NS::RetainPtr(
        MTL::TextureDescriptor::texture2DDescriptor(getMetalPixelFormat(), size.width, size.height, false));
    metalTexture = context.createMetalTexture(textureDescriptor);

    context.renderingStats().memTextures += getDataSize();
}

void Texture2D::createStorage(const void* data) noexcept {
    assert(metalTexture.get());

    MTL::Region region = MTL::Region::Make2D(0, 0, size.width, size.height);
    NS::UInteger bytesPerRow = size.width * getPixelStride();
    metalTexture->replaceRegion(region, 0, data, bytesPerRow);
    storageDirty = false;
    updateSamplerConfiguration();
}

void Texture2D::create() noexcept {
    if (!metalTexture.get()) {
        createObject();
    }
    if (storageDirty) {
        createStorage();
    }
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
    if (size_ == Size{0, 0}) {
        return;
    }

    if (!metalTexture.get() || storageDirty || size_ != size) {
        size = size_;

        // Create the texture object if we don't already have one
        if (!metalTexture.get()) {
            createObject();
        }

        createStorage(pixelData);

    } else {
        if (pixelData) {
            // Upload to existing memory
            uploadSubRegion(pixelData, size, 0, 0);
        }
    }
}

void Texture2D::uploadSubRegion(const void* pixelData, const Size& size_, uint16_t xOffset, uint16_t yOffset) noexcept {
    assert(metalTexture.get());
    assert(!samplerStateDirty);

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
