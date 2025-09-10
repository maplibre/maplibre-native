#pragma once

#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <mbgl/util/size.hpp>
#include <memory>

namespace mbgl {
namespace webgpu {

class Context;

class Texture2D : public gfx::Texture2D {
public:
    Texture2D(Context& context);
    ~Texture2D() override;

    Size getSize() const override { return size; }
    size_t getDataSize() const override { return dataSize; }
    size_t getPixelStride() const override { return pixelStride; }
    size_t numChannels() const override { return channelCount; }
    
    void upload(const void* data, Size size_, gfx::TextureChannelDataType type) override;
    void upload(const void* data, Size size_, gfx::TexturePixelType, gfx::TextureChannelDataType) override;
    void bind(gfx::TextureFilterType, gfx::TextureMipMapType, gfx::TextureWrapType) override;
    
    void setSamplerConfiguration(gfx::TextureFilterType filter,
                                gfx::TextureMipMapType mipmap,
                                gfx::TextureWrapType wrap) noexcept override;
    
    gfx::TextureFilterType getFilter() const noexcept override { return filter; }
    gfx::TextureMipMapType getMipmap() const noexcept override { return mipmap; }
    gfx::TextureWrapType getWrap() const noexcept override { return wrap; }
    
    // WebGPU specific
    WGPUTexture getTexture() const { return texture; }
    WGPUTextureView getTextureView() const { return textureView; }
    WGPUSampler getSampler() const { return sampler; }
    
private:
    void createTexture(Size size, gfx::TextureChannelDataType type);
    void createSampler();
    WGPUTextureFormat getWebGPUFormat(gfx::TextureChannelDataType type) const;
    
    Context& context;
    WGPUTexture texture = nullptr;
    WGPUTextureView textureView = nullptr;
    WGPUSampler sampler = nullptr;
    
    Size size{0, 0};
    size_t dataSize = 0;
    size_t pixelStride = 0;
    size_t channelCount = 0;
    
    gfx::TextureFilterType filter = gfx::TextureFilterType::Nearest;
    gfx::TextureMipMapType mipmap = gfx::TextureMipMapType::No;
    gfx::TextureWrapType wrap = gfx::TextureWrapType::Clamp;
    
    bool samplerDirty = true;
};

} // namespace webgpu
} // namespace mbgl