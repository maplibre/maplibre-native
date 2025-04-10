#pragma once

#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/util/image.hpp>

#include <memory>

namespace mbgl {

namespace gl {

class Context;

class Texture2D : public gfx::Texture2D {
public:
    Texture2D(gl::Context& context_);
    ~Texture2D() override;

public: // gfx::Texture2D
    Texture2D& setSamplerConfiguration(const SamplerState& samplerState) noexcept override;

    Texture2D& setFormat(gfx::TexturePixelType pixelFormat, gfx::TextureChannelDataType channelType) noexcept override;

    Texture2D& setSize(Size size_) noexcept override;

    Texture2D& setImage(std::shared_ptr<PremultipliedImage> image_) noexcept override;

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

public:
    /// @brief Get the OpenGL handle ID for the underlying resource
    /// @return GLuint
    platform::GLuint getTextureID() const noexcept;

    void updateSamplerConfiguration() noexcept;

    /// @brief Bind this texture to the specified texture unit
    /// @param location Location index of texture sampler in a shader
    /// @param textureUnit Unit to bind to. A maximum of gl::MaxActiveTextureUnits
    /// texture units are available for binding.
    void bind(int32_t location, int32_t textureUnit) noexcept;

    /// @brief Unbind the texture, if it was bound
    void unbind() noexcept;

private:
    void allocateTexture() noexcept;
    void updateTextureData(const void* data = nullptr) noexcept;

private:
    gl::Context& context;
    std::unique_ptr<gfx::TextureResource> textureResource{nullptr};

    SamplerState samplerState{};
    gfx::TexturePixelType pixelFormat{gfx::TexturePixelType::RGBA};
    gfx::TextureChannelDataType channelType{gfx::TextureChannelDataType::UnsignedByte};

    std::shared_ptr<PremultipliedImage> image{nullptr};
    Size size{0, 0};
    bool samplerStateDirty{false};
    bool storageDirty{false};

    int32_t boundTextureUnit{-1};
    int32_t boundLocation{-1};
};

} // namespace gl
} // namespace mbgl
