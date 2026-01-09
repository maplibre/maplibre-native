#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <webgpu/webgpu.h>

#include <algorithm>
#include <cstring>
#include <vector>

namespace {
constexpr uint32_t bytesPerRowAlignment = 256u;

struct UploadCopyData {
    const void* dataPtr = nullptr;
    size_t dataSize = 0u;
    uint32_t bytesPerRow = 0u;
    std::vector<uint8_t> staging;
};

UploadCopyData prepareCopyData(const void* pixelData, const mbgl::Size& regionSize, size_t pixelStride) {
    UploadCopyData copyData;

    if (!pixelData || regionSize.width == 0 || regionSize.height == 0) {
        return copyData;
    }

    const size_t rowStride = static_cast<size_t>(regionSize.width) * pixelStride;
    copyData.bytesPerRow = static_cast<uint32_t>(rowStride);
    copyData.dataSize = rowStride * regionSize.height;
    copyData.dataPtr = pixelData;

    if (regionSize.height > 1 && (rowStride % bytesPerRowAlignment) != 0) {
        const size_t alignedRowStride = ((rowStride + bytesPerRowAlignment - 1) / bytesPerRowAlignment) *
                                        bytesPerRowAlignment;
        copyData.bytesPerRow = static_cast<uint32_t>(alignedRowStride);
        copyData.staging.resize(alignedRowStride * regionSize.height, 0u);

        auto* dst = copyData.staging.data();
        const auto* src = static_cast<const uint8_t*>(pixelData);
        for (uint32_t row = 0; row < regionSize.height; ++row) {
            std::memcpy(dst + static_cast<size_t>(row) * alignedRowStride,
                        src + static_cast<size_t>(row) * rowStride,
                        rowStride);
        }

        copyData.dataPtr = copyData.staging.data();
        copyData.dataSize = copyData.staging.size();
    }

    return copyData;
}
} // namespace

namespace mbgl {
namespace webgpu {

Texture2D::Texture2D(Context& context_)
    : context(context_) {}

Texture2D::~Texture2D() {
    if (textureView) {
        wgpuTextureViewRelease(textureView);
        textureView = nullptr;
    }
    if (texture) {
        wgpuTextureRelease(texture);
        texture = nullptr;
    }
    if (sampler) {
        wgpuSamplerRelease(sampler);
        sampler = nullptr;
    }
}

gfx::Texture2D& Texture2D::setSamplerConfiguration(const gfx::Texture2D::SamplerState& samplerState_) noexcept {
    gfx::Texture2D::SamplerState safeState = samplerState_;
    if (safeState.maxAnisotropy < 1) {
        safeState.maxAnisotropy = 1;
    }
    samplerState = safeState;

    // Release old sampler if it exists
    if (sampler) {
        wgpuSamplerRelease(sampler);
        sampler = nullptr;
    }

    // Create new sampler with updated configuration
    WGPUSamplerDescriptor samplerDesc = {};
    WGPUStringView samplerLabel = {"Texture Sampler", strlen("Texture Sampler")};
    samplerDesc.label = samplerLabel;

    // Map filter modes
    samplerDesc.minFilter = (samplerState.filter == gfx::TextureFilterType::Linear) ? WGPUFilterMode_Linear
                                                                                    : WGPUFilterMode_Nearest;
    samplerDesc.magFilter = (samplerState.filter == gfx::TextureFilterType::Linear) ? WGPUFilterMode_Linear
                                                                                    : WGPUFilterMode_Nearest;
    samplerDesc.mipmapFilter = samplerState.mipmapped ? WGPUMipmapFilterMode_Linear : WGPUMipmapFilterMode_Nearest;

    // Map wrap modes
    auto mapWrapMode = [](gfx::TextureWrapType wrap) -> WGPUAddressMode {
        switch (wrap) {
            case gfx::TextureWrapType::Repeat:
                return WGPUAddressMode_Repeat;
            case gfx::TextureWrapType::Clamp:
                return WGPUAddressMode_ClampToEdge;
            default:
                return WGPUAddressMode_ClampToEdge;
        }
    };

    samplerDesc.addressModeU = mapWrapMode(samplerState.wrapU);
    samplerDesc.addressModeV = mapWrapMode(samplerState.wrapV);
    samplerDesc.addressModeW = WGPUAddressMode_ClampToEdge;
    samplerDesc.maxAnisotropy = static_cast<uint16_t>(samplerState.maxAnisotropy);

    // Create the sampler
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    if (device) {
        sampler = wgpuDeviceCreateSampler(device, &samplerDesc);
    }

    return *this;
}

gfx::Texture2D& Texture2D::setFormat(gfx::TexturePixelType pixelFormat_,
                                     gfx::TextureChannelDataType channelType_) noexcept {
    pixelFormat = pixelFormat_;
    channelType = channelType_;
    return *this;
}

gfx::Texture2D& Texture2D::setSize(Size size_) noexcept {
    if (size == size_) {
        return *this;
    }
    size = size_;
    dirty = true;
    if (sizeChangedCallback) {
        sizeChangedCallback(size);
    }
    return *this;
}

gfx::Texture2D& Texture2D::setImage(std::shared_ptr<PremultipliedImage> image_) noexcept {
    image = image_;
    if (image) {
        size = image->size;
        dirty = true;
    }
    return *this;
}

Texture2D& Texture2D::setUsage(uint32_t usageFlags_) noexcept {
    usageFlags = usageFlags_;
    return *this;
}

size_t Texture2D::getDataSize() const noexcept {
    return size.width * size.height * getPixelStride();
}

size_t Texture2D::getPixelStride() const noexcept {
    switch (pixelFormat) {
        case gfx::TexturePixelType::Alpha:
        case gfx::TexturePixelType::Luminance:
            switch (channelType) {
                case gfx::TextureChannelDataType::UnsignedByte:
                    return 1;
                case gfx::TextureChannelDataType::HalfFloat:
                    return 2;
                case gfx::TextureChannelDataType::Float:
                    return 4;
            }
            return 1;
        case gfx::TexturePixelType::RGBA:
            switch (channelType) {
                case gfx::TextureChannelDataType::UnsignedByte:
                    return 4;
                case gfx::TextureChannelDataType::HalfFloat:
                    return 8;
                case gfx::TextureChannelDataType::Float:
                    return 16;
            }
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
    // If texture already exists, don't recreate it
    if (texture && textureView) {
        return;
    }

    // Release old resources if they exist
    if (textureView) {
        wgpuTextureViewRelease(textureView);
        textureView = nullptr;
    }
    if (texture) {
        wgpuTextureRelease(texture);
        texture = nullptr;
    }

    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    if (!device || size.width == 0 || size.height == 0) {
        return;
    }

    // Determine WebGPU texture format based on pixel format
    WGPUTextureFormat format = WGPUTextureFormat_Undefined;
    uint32_t usage = usageFlags;
    switch (pixelFormat) {
        case gfx::TexturePixelType::Alpha:
            switch (channelType) {
                case gfx::TextureChannelDataType::UnsignedByte:
                    format = WGPUTextureFormat_R8Unorm;
                    break;
                case gfx::TextureChannelDataType::HalfFloat:
                    format = WGPUTextureFormat_R16Float;
                    usage |= WGPUTextureUsage_RenderAttachment;
                    break;
                case gfx::TextureChannelDataType::Float:
                    format = WGPUTextureFormat_R32Float;
                    usage |= WGPUTextureUsage_RenderAttachment;
                    break;
            }
            break;
        case gfx::TexturePixelType::Luminance:
            switch (channelType) {
                case gfx::TextureChannelDataType::UnsignedByte:
                    format = WGPUTextureFormat_R8Unorm;
                    break;
                case gfx::TextureChannelDataType::HalfFloat:
                    format = WGPUTextureFormat_R16Float;
                    usage |= WGPUTextureUsage_RenderAttachment;
                    break;
                case gfx::TextureChannelDataType::Float:
                    format = WGPUTextureFormat_R32Float;
                    usage |= WGPUTextureUsage_RenderAttachment;
                    break;
            }
            break;
        case gfx::TexturePixelType::Depth:
            format = WGPUTextureFormat_Depth32Float;
            usage |= WGPUTextureUsage_RenderAttachment;
            break;
        case gfx::TexturePixelType::Stencil:
#ifdef WGPUTextureFormat_Stencil8
            format = WGPUTextureFormat_Stencil8;
#else
            format = WGPUTextureFormat_Depth24PlusStencil8;
#endif
            usage |= WGPUTextureUsage_RenderAttachment;
            break;
        case gfx::TexturePixelType::RGBA:
        default:
            switch (channelType) {
                case gfx::TextureChannelDataType::UnsignedByte:
                    format = WGPUTextureFormat_RGBA8Unorm;
                    break;
                case gfx::TextureChannelDataType::HalfFloat:
                    format = WGPUTextureFormat_RGBA16Float;
                    usage |= WGPUTextureUsage_RenderAttachment;
                    break;
                case gfx::TextureChannelDataType::Float:
                    format = WGPUTextureFormat_RGBA32Float;
                    usage |= WGPUTextureUsage_RenderAttachment;
                    break;
            }
            break;
    }

    if (format == WGPUTextureFormat_Undefined) {
        format = WGPUTextureFormat_RGBA8Unorm;
    }

    nativeFormat = format;

    // Create texture descriptor
    WGPUTextureDescriptor textureDesc = {};
    WGPUStringView textureLabel = {"Texture2D", strlen("Texture2D")};
    textureDesc.label = textureLabel;
    textureDesc.size = {static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height), 1};
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.dimension = WGPUTextureDimension_2D;
    textureDesc.format = format;
    // Ensure color targets are renderable when requested via usage flags
    if (pixelFormat == gfx::TexturePixelType::RGBA) {
        usage |= WGPUTextureUsage_RenderAttachment;
    }
    textureDesc.usage = usage;

    // Create the texture
    texture = wgpuDeviceCreateTexture(device, &textureDesc);

    // Create texture view
    if (texture) {
        WGPUTextureViewDescriptor viewDesc = {};
        WGPUStringView viewLabel = {"Texture2D View", strlen("Texture2D View")};
        viewDesc.label = viewLabel;
        viewDesc.format = format;
        viewDesc.dimension = WGPUTextureViewDimension_2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = WGPUTextureAspect_All;

        textureView = wgpuTextureCreateView(texture, &viewDesc);
    }

    // Create default sampler if not already created
    if (!sampler) {
        setSamplerConfiguration(samplerState);
    }
}

void Texture2D::setSizeChangedCallback(std::function<void(const Size&)> callback) noexcept {
    sizeChangedCallback = std::move(callback);
}

void Texture2D::upload(const void* pixelData, const Size& size_) noexcept {
    size = size_;

    // Create texture if it doesn't exist
    if (!texture) {
        create();
    }

    if (!texture || !pixelData) {
        return;
    }

    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());
    if (!device || !queue) {
        return;
    }

    const auto copyData = prepareCopyData(pixelData, size, getPixelStride());
    if (!copyData.dataPtr) {
        return;
    }

    WGPUTexelCopyTextureInfo destination = {};
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = {0, 0, 0};
    destination.aspect = WGPUTextureAspect_All;

    WGPUTexelCopyBufferLayout dataLayout = {};
    dataLayout.offset = 0;
    dataLayout.bytesPerRow = copyData.bytesPerRow;
    dataLayout.rowsPerImage = static_cast<uint32_t>(size.height);

    WGPUExtent3D writeSize = {static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height), 1};

    wgpuQueueWriteTexture(queue, &destination, copyData.dataPtr, copyData.dataSize, &dataLayout, &writeSize);
}

void Texture2D::upload() noexcept {
    if (image) {
        upload(image->data.get(), image->size);
        dirty = false;
    }
}

void Texture2D::uploadSubRegion(const void* pixelData,
                                const Size& regionSize,
                                uint16_t xOffset,
                                uint16_t yOffset) noexcept {
    if (!texture || !pixelData) {
        return;
    }

    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());
    if (!queue) {
        return;
    }

    const auto copyData = prepareCopyData(pixelData, regionSize, getPixelStride());
    if (!copyData.dataPtr) {
        return;
    }

    WGPUTexelCopyTextureInfo destination = {};
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = {xOffset, yOffset, 0};
    destination.aspect = WGPUTextureAspect_All;

    WGPUTexelCopyBufferLayout dataLayout = {};
    dataLayout.offset = 0;
    dataLayout.bytesPerRow = copyData.bytesPerRow;
    dataLayout.rowsPerImage = static_cast<uint32_t>(regionSize.height);

    WGPUExtent3D writeSize = {static_cast<uint32_t>(regionSize.width), static_cast<uint32_t>(regionSize.height), 1};

    wgpuQueueWriteTexture(queue, &destination, copyData.dataPtr, copyData.dataSize, &dataLayout, &writeSize);
}

} // namespace webgpu
} // namespace mbgl
