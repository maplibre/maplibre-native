#pragma once
#include <mbgl/gfx/types.hpp>
#include <mbgl/util/image.hpp>

#include <cstddef>
#include <vector>

namespace mbgl {

class Size;

namespace gfx {

class Context;
class UploadPass;
class TextureResource;

constexpr int32_t MaxActiveTextureUnits = 8;

class Texture2D {
public:
    struct SamplerState {
        /// Minification and magnification filter within a single mip level
        TextureFilterType filter{TextureFilterType::Nearest};
        /// Wrapping behavior along U coordinate
        TextureWrapType wrapU{TextureWrapType::Clamp};
        /// Wrapping behavior along V coordinate
        TextureWrapType wrapV{TextureWrapType::Clamp};

        uint8_t maxAnisotropy{1};
        bool mipmapped{false};
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

    /// @brief Sets the internal image
    /// @param image_ Image data to transfer
    virtual Texture2D& setImage(std::shared_ptr<PremultipliedImage> image_) noexcept = 0;

    /// @brief Get the pixel format of the texture
    /// @return Pixel format of the texture
    virtual TexturePixelType getFormat() const noexcept = 0;

    /// @brief Get the size of the texture
    /// @return Size of the texture
    virtual Size getSize() const noexcept = 0;

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

    /// @brief Create the texture with default initialized memory.
    /// @note Be sure to configure a valid size and format.
    virtual void create() noexcept = 0;

    /// @brief Upload image data to the texture resource
    /// @param pixelData Image data to transfer
    virtual void upload(const void* pixelData, const Size& size_) noexcept = 0;

    /// @brief Upload image data to the texture resource
    /// @tparam Image Image object type
    /// @param img Image to transfer
    template <typename Image>
    void upload(const Image& img) noexcept {
        setFormat(Image::channels == 1 ? gfx::TexturePixelType::Alpha : gfx::TexturePixelType::RGBA,
                  gfx::TextureChannelDataType::UnsignedByte);
        upload(img.data ? img.data.get() : nullptr, img.size);
    }

    /// @brief Upload a subregion of the texture resource
    /// @param pixelData image pixel data pointer
    /// @param size image dimensions
    /// @param xOffset destination x coordinate
    /// @param yOffset destination y coordinate
    virtual void uploadSubRegion(const void* pixelData,
                                 const Size& size,
                                 uint16_t xOffset,
                                 uint16_t yOffset) noexcept = 0;

    /// @brief Upload a subregion of the texture resource
    /// @tparam Image Image object type
    /// @param img Image to upload
    /// @param xOffset Destination x coordinate
    /// @param yOffset Destination y coordinate
    template <typename Image>
    void uploadSubRegion(const Image& img, uint16_t xOffset, uint16_t yOffset) noexcept {
        assert(Image::channels == numChannels());
        assert(Image::channels == getPixelStride());
        assert(img.size.width + xOffset <= getSize().width);
        assert(img.size.height + yOffset <= getSize().height);
        uploadSubRegion(img.data ? img.data.get() : nullptr, img.size, xOffset, yOffset);
    }

    /// @brief Upload staged image data if present and required.
    /// @see needsUpload
    virtual void upload() noexcept = 0;

    /// @brief Check whether the texture needs upload
    /// @return bool
    virtual bool needsUpload() const noexcept = 0;
};

} // namespace gfx
} // namespace mbgl
