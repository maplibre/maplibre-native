#pragma once

#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>
#include <mbgl/util/image.hpp>

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLPixelFormat.hpp>
#include <Metal/MTLTexture.hpp>

#include <memory>

namespace mbgl {
namespace mtl {

class Context;
class RenderPass;

class Texture2D : public gfx::Texture2D {
public:
    Texture2D(Context& context_);
    ~Texture2D() override;

    gfx::Texture2D& setSamplerConfiguration(const SamplerState&) noexcept override;

    gfx::Texture2D& setFormat(gfx::TexturePixelType, gfx::TextureChannelDataType) noexcept override;

    gfx::Texture2D& setSize(Size size_) noexcept override;

    gfx::Texture2D& setImage(std::shared_ptr<PremultipliedImage>) noexcept override;

    gfx::TexturePixelType getFormat() const noexcept override { return pixelFormat; }

    Size getSize() const noexcept override { return size; }

    size_t getDataSize() const noexcept override;

    size_t getPixelStride() const noexcept override;

    size_t numChannels() const noexcept override;

    void create() noexcept override;

    void upload(const void* pixelData, const Size& size_) noexcept override;
    void uploadSubRegion(const void* pixelData, const Size& size, uint16_t xOffset, uint16_t yOffset) noexcept override;
    void upload() noexcept override;

    bool needsUpload() const noexcept override { return !!image; };

    gfx::Texture2D& setUsage(MTL::TextureUsage usage_) noexcept;

    MTL::Texture* getMetalTexture() const noexcept;

    void updateSamplerConfiguration() noexcept;

    /// @brief Bind this texture to the specified location
    /// @param renderPass Render pass on which the texture will be assign
    /// @param location Location index of texture sampler in a shader
    void bind(RenderPass& renderPass, int32_t location) noexcept;

    /// @brief Unbind the texture, if it was bound
    /// @param renderPass Render pass from which the texture will be removed
    /// @param location Location index of texture sampler in a shader
    void unbind(RenderPass& renderPass, int32_t location) noexcept;

private:
    MTL::PixelFormat getMetalPixelFormat() const noexcept;
    void createMetalTexture() noexcept;

    Context& context;
    MTLTexturePtr metalTexture;
    MTLSamplerStatePtr metalSamplerState;

    Size size{0, 0};
    gfx::TexturePixelType pixelFormat{gfx::TexturePixelType::RGBA};
    gfx::TextureChannelDataType channelType{gfx::TextureChannelDataType::UnsignedByte};
    MTL::TextureUsage usage{MTL::TextureUsageShaderRead};
    SamplerState samplerState{};

    std::shared_ptr<PremultipliedImage> image{nullptr};
    bool textureDirty{true};
    bool samplerStateDirty{true};
};

} // namespace mtl
} // namespace mbgl
