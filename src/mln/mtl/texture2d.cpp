#include <mln/mtl/texture2d.hpp>
#include <mln/mtl/context.hpp>
#include <mln/mtl/render_pass.hpp>
#include <mln/mtl/upload_pass.hpp>

#include <Metal/MTLDevice.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>
#include <Metal/MTLSampler.hpp>

#include <algorithm>

namespace mln {
namespace mtl {

Texture2D::Texture2D(Context& context_)
    : context(context_) {}

Texture2D::~Texture2D() {
    destroyMetalTexture();
}

gfx::Texture2D& Texture2D::setSamplerConfiguration(const SamplerState& samplerState_) noexcept {
    if (samplerState.filter == samplerState_.filter && samplerState.wrapU == samplerState_.wrapU &&
        samplerState.wrapV == samplerState_.wrapV && samplerState.mipmapped == samplerState_.mipmapped &&
        samplerState.maxAnisotropy == samplerState_.maxAnisotropy) {
        return *this;
    }

    if (samplerState.mipmapped != samplerState_.mipmapped) {
        // The mip chain is part of the texture itself
        destroyMetalTexture();
        textureDirty = true;
    }
    samplerState = samplerState_;
    samplerStateDirty = true;
    return *this;
}

gfx::Texture2D& Texture2D::setFormat(gfx::TexturePixelType pixelFormat_,
                                     gfx::TextureChannelDataType channelType_) noexcept {
    if (pixelFormat_ == pixelFormat && channelType_ == channelType) {
        return *this;
    }

    destroyMetalTexture();

    pixelFormat = pixelFormat_;
    channelType = channelType_;
    textureDirty = true;
    return *this;
}

gfx::Texture2D& Texture2D::setSize(mln::Size size_) noexcept {
    if (size_ == size) {
        return *this;
    }

    destroyMetalTexture();

    size = size_;
    textureDirty = true;
    return *this;
}

gfx::Texture2D& Texture2D::setImage(std::shared_ptr<PremultipliedImage> image_) noexcept {
    image = std::move(image_);
    return *this;
}

size_t Texture2D::getDataSize() const noexcept {
    return size.width * size.height * getPixelStride(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
}

size_t Texture2D::getStorageSize() const noexcept {
    size_t bytes = getDataSize(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
    if (!samplerState.mipmapped) {
        return bytes;
    }
    // Every level of the chain, down to 1 x 1: a third more than the base level
    for (auto level = size; level.width > 1 || level.height > 1;) {
        level = {std::max(level.width / 2, 1u), std::max(level.height / 2, 1u)};
        bytes += level.width * level.height * getPixelStride(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
    }
    return bytes;
}

size_t Texture2D::getPixelStride() const noexcept {
    switch (channelType) {
        case gfx::TextureChannelDataType::UnsignedByte:
            return 1 * numChannels(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
        case gfx::TextureChannelDataType::HalfFloat:
            return 2 * numChannels(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
        case gfx::TextureChannelDataType::Float:
            return 4 * numChannels(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
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

MTL::PixelFormat Texture2D::getMetalPixelFormat() const noexcept {
    // On iOS simulator and x86-64, we need to use the combined depth/stencil format.  If the depth and stencil
    // formats are both set on a render pipeline, they have to be identical or we'll get, e.g.:
    //     validateWithDevice:4343: failed assertion `Render Pipeline Descriptor Validation
    //                              depthAttachmentPixelFormat (MTLPixelFormatDepth32Float) and
    //                              stencilAttachmentPixelFormat (MTLPixelFormatStencil8) must match.

#if TARGET_OS_SIMULATOR || defined(__x86_64__)
    if ((channelType == gfx::TextureChannelDataType::Float ||
         channelType == gfx::TextureChannelDataType::UnsignedByte) &&
        (pixelFormat == gfx::TexturePixelType::Depth || pixelFormat == gfx::TexturePixelType::Stencil)) {
        return MTL::PixelFormatDepth32Float_Stencil8;
    }
#endif

    switch (channelType) {
        case gfx::TextureChannelDataType::UnsignedByte:
            switch (pixelFormat) {
                case gfx::TexturePixelType::RGBA:
                    return MTL::PixelFormat::PixelFormatRGBA8Unorm;
                case gfx::TexturePixelType::Alpha:
                    return MTL::PixelFormat::PixelFormatA8Unorm;
                case gfx::TexturePixelType::Stencil:
                    return MTL::PixelFormat::PixelFormatStencil8;
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
        case gfx::TextureChannelDataType::Float:
            switch (pixelFormat) {
                case gfx::TexturePixelType::RGBA:
                    return MTL::PixelFormat::PixelFormatRGBA32Float;
                case gfx::TexturePixelType::Alpha:
                    return MTL::PixelFormat::PixelFormatR32Float;
                case gfx::TexturePixelType::Depth:
                    return MTL::PixelFormat::PixelFormatDepth32Float;
                default:
                    assert(false);
                    return MTL::PixelFormat::PixelFormatInvalid;
            }
        default:
            assert(false);
            return MTL::PixelFormat::PixelFormatInvalid;
    }
}

void Texture2D::createMetalTexture() {
    if (size == Size{0, 0}) {
        return;
    }

    destroyMetalTexture();

    const auto format = getMetalPixelFormat();
    if (format == MTL::PixelFormat::PixelFormatInvalid) {
        return;
    }

    // Create a new texture object
    if (auto textureDescriptor = NS::RetainPtr(
            MTL::TextureDescriptor::texture2DDescriptor(format, size.width, size.height, samplerState.mipmapped))) {
        textureDescriptor->setUsage(usage);
#if TARGET_OS_SIMULATOR || defined(__x86_64__)
        switch (format) {
            case MTL::PixelFormatDepth16Unorm:
            case MTL::PixelFormatDepth32Float:
            case MTL::PixelFormatStencil8:
            case MTL::PixelFormatDepth24Unorm_Stencil8:
            case MTL::PixelFormatDepth32Float_Stencil8:
            case MTL::PixelFormatX32_Stencil8:
            case MTL::PixelFormatX24_Stencil8:
                // On iOS simulator and x86-64, the default shared mode is invalid for depth and stencil textures.
                //  'Texture Descriptor Validation MTLTextureDescriptor: Depth, Stencil, DepthStencil
                //   textures cannot be allocated with MTLStorageModeShared on this device.
                textureDescriptor->setStorageMode(MTL::StorageMode::StorageModePrivate);
                break;
            default:
                break;
        }
#endif
        metalTexture = context.createMetalTexture(std::move(textureDescriptor));
        if (!metalTexture) {
            throw std::bad_alloc();
        }
    }

    if (metalTexture) {
        textureDirty = false;
        trackedBytes = getStorageSize();
        context.threadSafeAccessRenderingStats([&](gfx::RenderingStats& stats) {
            stats.numCreatedTextures++;
            stats.numActiveTextures++;
            stats.memTextures += trackedBytes;
        });
    }
}

void Texture2D::create() {
    if (textureDirty) {
        createMetalTexture();
    }
    if (samplerStateDirty) {
        updateSamplerConfiguration();
    }
}

void Texture2D::destroyMetalTexture() noexcept {
    if (!metalTexture) {
        return;
    }

    metalTexture.reset();
    context.threadSafeAccessRenderingStats([&](gfx::RenderingStats& stats) {
        stats.numActiveTextures--;
        stats.memTextures -= trackedBytes;
    });
    trackedBytes = 0;
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

void Texture2D::updateSamplerConfiguration() {
    auto samplerDescriptor = NS::TransferPtr(MTL::SamplerDescriptor::alloc()->init());
    samplerDescriptor->setMinFilter(samplerState.filter == gfx::TextureFilterType::Nearest
                                        ? MTL::SamplerMinMagFilterNearest
                                        : MTL::SamplerMinMagFilterLinear);
    samplerDescriptor->setMagFilter(samplerState.filter == gfx::TextureFilterType::Nearest
                                        ? MTL::SamplerMinMagFilterNearest
                                        : MTL::SamplerMinMagFilterLinear);
    // Trilinear between the levels of a mip chain, and anisotropic along a stretched footprint
    if (samplerState.mipmapped) {
        samplerDescriptor->setMipFilter(MTL::SamplerMipFilterLinear);
    }
    samplerDescriptor->setMaxAnisotropy(std::max<NS::UInteger>(samplerState.maxAnisotropy, 1));
    samplerDescriptor->setSAddressMode(samplerState.wrapU == gfx::TextureWrapType::Clamp
                                           ? MTL::SamplerAddressModeClampToEdge
                                           : MTL::SamplerAddressModeRepeat);
    samplerDescriptor->setTAddressMode(samplerState.wrapV == gfx::TextureWrapType::Clamp
                                           ? MTL::SamplerAddressModeClampToEdge
                                           : MTL::SamplerAddressModeRepeat);
    metalSamplerState = context.createMetalSamplerState(samplerDescriptor);
    if (!metalSamplerState) {
        throw std::bad_alloc();
    }

    samplerStateDirty = false;
}

void Texture2D::bind(RenderPass& renderPass, int32_t location) {
    assert(!textureDirty);

    // Update the sampler state if it was changed after resource creation
    if (samplerStateDirty) {
        updateSamplerConfiguration();
    }

    // Bind to BOTH vertex and fragment stages to support vertex shader texture sampling (e.g., terrain DEM)
    renderPass.setVertexTexture(metalTexture, location);
    renderPass.setVertexSamplerState(metalSamplerState, location);
    renderPass.setFragmentTexture(metalTexture, location);
    renderPass.setFragmentSamplerState(metalSamplerState, location);

    context.threadSafeAccessRenderingStats([&](gfx::RenderingStats& stats) { stats.numTextureBindings++; });
}

void Texture2D::unbind(RenderPass&, int32_t /*location*/) noexcept {
    context.threadSafeAccessRenderingStats([&](gfx::RenderingStats& stats) { stats.numTextureBindings--; });
}

void Texture2D::upload(const void* pixelData, const Size& size_) {
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

    const MTL::Region region = MTL::Region::Make2D(xOffset, yOffset, size_.width, size_.height);
    const NS::UInteger bytesPerRow = size_.width * getPixelStride();
    metalTexture->replaceRegion(region, 0, pixelData, bytesPerRow);
    context.threadSafeAccessRenderingStats([&](gfx::RenderingStats& stats) {
        stats.numTextureUpdates++;
        stats.textureUpdateBytes += bytesPerRow * size_.height;
    });
}

void Texture2D::upload() {
    if (image && image->valid()) {
        setFormat(gfx::TexturePixelType::RGBA, gfx::TextureChannelDataType::UnsignedByte);
        upload(image->data.get(), image->size);
        image.reset();
    }
}

} // namespace mtl
} // namespace mln
