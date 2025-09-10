#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

Texture2D::Texture2D(Context& context_)
    : context(context_) {
}

Texture2D::~Texture2D() {
    if (sampler) {
        // wgpuSamplerRelease(sampler);
    }
    if (textureView) {
        // wgpuTextureViewRelease(textureView);
    }
    if (texture) {
        // wgpuTextureDestroy(texture);
        // wgpuTextureRelease(texture);
    }
}

void Texture2D::upload(const void* data, Size size_, gfx::TextureChannelDataType type) {
    upload(data, size_, gfx::TexturePixelType::RGBA, type);
}

void Texture2D::upload(const void* data, Size size_, gfx::TexturePixelType pixelType, gfx::TextureChannelDataType channelType) {
    if (!data || size_.width == 0 || size_.height == 0) {
        return;
    }
    
    // Create texture if needed or if size changed
    if (!texture || size != size_) {
        createTexture(size_, channelType);
    }
    
    if (!texture) {
        Log::Error(Event::Render, "Failed to create WebGPU texture");
        return;
    }
    
    auto* impl = context.getImpl();
    if (!impl) {
        return;
    }
    
    WGPUDevice device = impl->getDevice();
    WGPUQueue queue = impl->getQueue();
    if (!device || !queue) {
        return;
    }
    
    // Calculate data size
    switch (channelType) {
        case gfx::TextureChannelDataType::UnsignedByte:
            pixelStride = 4; // RGBA
            channelCount = 4;
            break;
        case gfx::TextureChannelDataType::HalfFloat:
            pixelStride = 8; // RGBA16F
            channelCount = 4;
            break;
        case gfx::TextureChannelDataType::Float:
            pixelStride = 16; // RGBA32F
            channelCount = 4;
            break;
    }
    
    dataSize = size_.width * size_.height * pixelStride;
    
    // Write texture data
    WGPUImageCopyTexture destination = {};
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = {0, 0, 0};
    destination.aspect = WGPUTextureAspect_All;
    
    WGPUTextureDataLayout dataLayout = {};
    dataLayout.offset = 0;
    dataLayout.bytesPerRow = size_.width * pixelStride;
    dataLayout.rowsPerImage = size_.height;
    
    WGPUExtent3D writeSize = {};
    writeSize.width = size_.width;
    writeSize.height = size_.height;
    writeSize.depthOrArrayLayers = 1;
    
    // wgpuQueueWriteTexture(queue, &destination, data, dataSize, &dataLayout, &writeSize);
}

void Texture2D::bind(gfx::TextureFilterType filter_, gfx::TextureMipMapType mipmap_, gfx::TextureWrapType wrap_) {
    setSamplerConfiguration(filter_, mipmap_, wrap_);
}

void Texture2D::setSamplerConfiguration(gfx::TextureFilterType filter_,
                                       gfx::TextureMipMapType mipmap_,
                                       gfx::TextureWrapType wrap_) noexcept {
    if (filter != filter_ || mipmap != mipmap_ || wrap != wrap_) {
        filter = filter_;
        mipmap = mipmap_;
        wrap = wrap_;
        samplerDirty = true;
    }
    
    if (samplerDirty) {
        createSampler();
        samplerDirty = false;
    }
}

void Texture2D::createTexture(Size size_, gfx::TextureChannelDataType type) {
    // Clean up old resources
    if (textureView) {
        // wgpuTextureViewRelease(textureView);
        textureView = nullptr;
    }
    if (texture) {
        // wgpuTextureDestroy(texture);
        // wgpuTextureRelease(texture);
        texture = nullptr;
    }
    
    size = size_;
    
    auto* impl = context.getImpl();
    if (!impl) {
        return;
    }
    
    WGPUDevice device = impl->getDevice();
    if (!device) {
        return;
    }
    
    WGPUTextureDescriptor textureDesc = {};
    textureDesc.label = "MapLibre Texture2D";
    textureDesc.size.width = size.width;
    textureDesc.size.height = size.height;
    textureDesc.size.depthOrArrayLayers = 1;
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.dimension = WGPUTextureDimension_2D;
    textureDesc.format = getWebGPUFormat(type);
    textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    
    // texture = wgpuDeviceCreateTexture(device, &textureDesc);
    
    if (texture) {
        // Create texture view
        WGPUTextureViewDescriptor viewDesc = {};
        viewDesc.label = "MapLibre Texture2D View";
        viewDesc.format = textureDesc.format;
        viewDesc.dimension = WGPUTextureViewDimension_2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = WGPUTextureAspect_All;
        
        // textureView = wgpuTextureCreateView(texture, &viewDesc);
    }
}

void Texture2D::createSampler() {
    if (sampler) {
        // wgpuSamplerRelease(sampler);
        sampler = nullptr;
    }
    
    auto* impl = context.getImpl();
    if (!impl) {
        return;
    }
    
    WGPUDevice device = impl->getDevice();
    if (!device) {
        return;
    }
    
    WGPUSamplerDescriptor samplerDesc = {};
    samplerDesc.label = "MapLibre Sampler";
    
    // Set filter modes
    switch (filter) {
        case gfx::TextureFilterType::Nearest:
            samplerDesc.magFilter = WGPUFilterMode_Nearest;
            samplerDesc.minFilter = WGPUFilterMode_Nearest;
            break;
        case gfx::TextureFilterType::Linear:
            samplerDesc.magFilter = WGPUFilterMode_Linear;
            samplerDesc.minFilter = WGPUFilterMode_Linear;
            break;
    }
    
    // Set mipmap mode
    switch (mipmap) {
        case gfx::TextureMipMapType::No:
            samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Nearest;
            break;
        case gfx::TextureMipMapType::Nearest:
            samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Nearest;
            break;
        case gfx::TextureMipMapType::Linear:
            samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
            break;
    }
    
    // Set wrap modes
    WGPUAddressMode addressMode = WGPUAddressMode_ClampToEdge;
    switch (wrap) {
        case gfx::TextureWrapType::Clamp:
            addressMode = WGPUAddressMode_ClampToEdge;
            break;
        case gfx::TextureWrapType::Repeat:
            addressMode = WGPUAddressMode_Repeat;
            break;
        case gfx::TextureWrapType::MirrorRepeat:
            addressMode = WGPUAddressMode_MirrorRepeat;
            break;
    }
    
    samplerDesc.addressModeU = addressMode;
    samplerDesc.addressModeV = addressMode;
    samplerDesc.addressModeW = addressMode;
    
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = 100.0f;
    samplerDesc.maxAnisotropy = 1;
    
    // sampler = wgpuDeviceCreateSampler(device, &samplerDesc);
}

WGPUTextureFormat Texture2D::getWebGPUFormat(gfx::TextureChannelDataType type) const {
    switch (type) {
        case gfx::TextureChannelDataType::UnsignedByte:
            return WGPUTextureFormat_RGBA8Unorm;
        case gfx::TextureChannelDataType::HalfFloat:
            return WGPUTextureFormat_RGBA16Float;
        case gfx::TextureChannelDataType::Float:
            return WGPUTextureFormat_RGBA32Float;
        default:
            return WGPUTextureFormat_RGBA8Unorm;
    }
}

} // namespace webgpu
} // namespace mbgl