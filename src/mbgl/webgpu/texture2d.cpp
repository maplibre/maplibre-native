#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/backend_impl.hpp>

namespace mbgl {
namespace webgpu {

Texture2D::Texture2D(Context& context_)
    : context(context_) {
}

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
    samplerState = samplerState_;
    
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
    samplerDesc.minFilter = (samplerState.filter == gfx::TextureFilterType::Linear) 
        ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
    samplerDesc.magFilter = (samplerState.filter == gfx::TextureFilterType::Linear)
        ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
    samplerDesc.mipmapFilter = samplerState.mipmapped
        ? WGPUMipmapFilterMode_Linear : WGPUMipmapFilterMode_Nearest;
    
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
    
    // Create the sampler
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    if (device) {
        sampler = wgpuDeviceCreateSampler(device, &samplerDesc);
    }
    
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
        dirty = true;
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
    WGPUTextureFormat format;
    switch (pixelFormat) {
        case gfx::TexturePixelType::Alpha:
            format = WGPUTextureFormat_R8Unorm;
            break;
        case gfx::TexturePixelType::Luminance:
            format = WGPUTextureFormat_R8Unorm;
            break;
        case gfx::TexturePixelType::RGBA:
        default:
            format = WGPUTextureFormat_RGBA8Unorm;
            break;
    }
    
    // Create texture descriptor
    WGPUTextureDescriptor textureDesc = {};
    WGPUStringView textureLabel = {"Texture2D", strlen("Texture2D")};
    textureDesc.label = textureLabel;
    textureDesc.size = {
        static_cast<uint32_t>(size.width),
        static_cast<uint32_t>(size.height),
        1
    };
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.dimension = WGPUTextureDimension_2D;
    textureDesc.format = format;
    textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    
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
    
    // Write texture data using queue
    WGPUTexelCopyTextureInfo destination = {};
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = {0, 0, 0};
    destination.aspect = WGPUTextureAspect_All;
    
    WGPUTexelCopyBufferLayout dataLayout = {};
    dataLayout.offset = 0;
    dataLayout.bytesPerRow = static_cast<uint32_t>(size.width * getPixelStride());
    dataLayout.rowsPerImage = static_cast<uint32_t>(size.height);
    
    WGPUExtent3D writeSize = {
        static_cast<uint32_t>(size.width),
        static_cast<uint32_t>(size.height),
        1
    };
    
    wgpuQueueWriteTexture(
        queue,
        &destination,
        pixelData,
        getDataSize(),
        &dataLayout,
        &writeSize
    );
}

void Texture2D::upload() noexcept {
    if (image) {
        upload(image->data.get(), image->size);
        dirty = false;
    }
}

void Texture2D::uploadSubRegion(const void* pixelData, const Size& regionSize, uint16_t xOffset, uint16_t yOffset) noexcept {
    if (!texture || !pixelData) {
        return;
    }
    
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());
    if (!queue) {
        return;
    }
    
    // Write texture subregion data using queue
    WGPUTexelCopyTextureInfo destination = {};
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = {xOffset, yOffset, 0};
    destination.aspect = WGPUTextureAspect_All;
    
    WGPUTexelCopyBufferLayout dataLayout = {};
    dataLayout.offset = 0;
    dataLayout.bytesPerRow = static_cast<uint32_t>(regionSize.width * getPixelStride());
    dataLayout.rowsPerImage = static_cast<uint32_t>(regionSize.height);
    
    WGPUExtent3D writeSize = {
        static_cast<uint32_t>(regionSize.width),
        static_cast<uint32_t>(regionSize.height),
        1
    };
    
    size_t dataSize = regionSize.width * regionSize.height * getPixelStride();
    
    wgpuQueueWriteTexture(
        queue,
        &destination,
        pixelData,
        dataSize,
        &dataLayout,
        &writeSize
    );
}

} // namespace webgpu
} // namespace mbgl
