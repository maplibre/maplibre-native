#pragma once
#include <mbgl/gfx/types.hpp>
#include <mbgl/util/image.hpp>

#include <cstddef>
#include <vector>

namespace mbgl {

class Size;

namespace gfx {

constexpr int32_t MaxActiveTextureUnits = 8;

class Context;
class UploadPass;
class TextureResource;

class Texture2D {
public:
    struct SamplerState {
        /// Minification filter within a single mip level
        TextureFilterType minification{TextureFilterType::Nearest};
        /// Magnification filter within a single mip level
        TextureFilterType magnification{TextureFilterType::Nearest};
        /// Wrapping behavior along U coordinate
        TextureWrapType wrapU{TextureWrapType::Clamp};
        /// Wrapping behavior along V coordinate
        TextureWrapType wrapV{TextureWrapType::Clamp};
    };

public:
    virtual ~Texture2D() = default;

    /// @brief Set the sampler configuration to be used for shader access of this
    /// texture.
    /// @param samplerState Sampler state configuration
    /// @return this
    virtual Texture2D& setSamplerConfiguration(const SamplerState& samplerState) noexcept = 0;

    /// @brief Set the texture pixel and data format
    /// @param pixelFormat Pixel format to use
    /// @param channelType Data type of each color channel
    /// @return this
    virtual Texture2D& setFormat(TexturePixelType pixelFormat, TextureChannelDataType channelType) noexcept = 0;

    /// @brief Set the size of the texture
    /// @param size_ Size of the texture
    /// @return this
    virtual Texture2D& setSize(Size size_) noexcept = 0;

    /// @brief Determine the size of the buffer backing this texture
    /// as configured, in bytes.
    /// @return Size in bytes
    virtual size_t getDataSize() const noexcept = 0;

    /// @brief Returns the size, in bytes, of a pixel in the texture.
    /// @return Size of a pixel, in bytes
    virtual size_t getPixelStride() const noexcept = 0;

    /// @brief Returns the number of color channels in the texture.
    /// @return Channel count
    virtual size_t numChannels() const noexcept = 0;

    /// @brief Create the texture using the provided buffer.
    /// @param pixelData Buffer of bytes to initialize the texture with
    virtual void create(const std::vector<uint8_t>& pixelData, gfx::UploadPass& uploadPass) noexcept = 0;

    /// @brief Create the texture with default initialized memory.
    virtual void create() noexcept = 0;

    /// @brief Upload image data to the texture resource
    /// @param image Image data to transfer
    /// @param uploadPass Upload pass to orchestrate upload
    virtual void upload(const PremultipliedImage& image, gfx::UploadPass& uploadPass) const noexcept = 0;

    /// @brief Get the underlying GL texture resource
    /// @note: Compat with legacy textures, to be refactored
    /// @return gfx::TextureResource
    virtual gfx::TextureResource& getResource() = 0;
};

} // namespace gfx
} // namespace mbgl
