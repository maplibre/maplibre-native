#include <mbgl/gl/texture2d.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/enum.hpp>
#include <mbgl/platform/gl_functions.hpp>

#include <mbgl/gl/texture_resource.hpp>
#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/upload_pass.hpp>

namespace mbgl {
namespace gl {

Texture2D::Texture2D(gl::Context& context_)
    : context(context_) {}

Texture2D::~Texture2D() {}

Texture2D& Texture2D::setSamplerConfiguration(const SamplerState& samplerState_) noexcept {
    samplerState = samplerState_;
    samplerStateDirty = textureResource != nullptr;
    return *this;
}

Texture2D& Texture2D::setFormat(gfx::TexturePixelType pixelFormat_, gfx::TextureChannelDataType channelType_) noexcept {
    if (pixelFormat_ == pixelFormat && channelType_ == channelType) {
        return *this;
    }

    assert(!textureResource);
    pixelFormat = pixelFormat_;
    channelType = channelType_;
    return *this;
}

Texture2D& Texture2D::setSize(mbgl::Size size_) noexcept {
    size = size_;
    return *this;
}

Texture2D& Texture2D::setImage(std::shared_ptr<PremultipliedImage> image_) noexcept {
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
        default:
            return 0;
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

void Texture2D::createObject() noexcept {
    // Create a new texture object
    assert(!textureResource);
    auto obj = context.createUniqueTexture();
    const auto storageSize = gl::TextureResource::getStorageSize(size, pixelFormat, channelType);
    context.renderingStats().memTextures += storageSize;

    // @TODO: TextureResource is still needed while we have legacy rendering pathways
    textureResource = std::make_unique<gl::TextureResource>(std::move(obj), storageSize);
}

void Texture2D::createStorage(const void* data) noexcept {
    assert(textureResource);

    // Create backing storage for our texture object
    using namespace platform;

    // Bind to TU 0 and upload
    context.activeTextureUnit = 0;
    context.texture[0] = getTextureID();
    updateSamplerConfiguration();
    MBGL_CHECK_ERROR(glTexImage2D(GL_TEXTURE_2D,
                                  0,
                                  Enum<gfx::TexturePixelType>::to(pixelFormat),
                                  size.width,
                                  size.height,
                                  0,
                                  Enum<gfx::TexturePixelType>::to(pixelFormat),
                                  Enum<gfx::TextureChannelDataType>::to(channelType),
                                  data));
}

void Texture2D::create() noexcept {
    createObject();
    createStorage();
}

platform::GLuint Texture2D::getTextureID() const noexcept {
    return static_cast<gl::TextureResource&>(*textureResource).texture;
}

void Texture2D::updateSamplerConfiguration() noexcept {
    using namespace platform;
    samplerStateDirty = false;

    MBGL_CHECK_ERROR(
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER,
                        samplerState.minification == gfx::TextureFilterType::Nearest ? GL_NEAREST : GL_LINEAR));
    MBGL_CHECK_ERROR(
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER,
                        samplerState.magnification == gfx::TextureFilterType::Nearest ? GL_NEAREST : GL_LINEAR));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D,
                                     GL_TEXTURE_WRAP_S,
                                     samplerState.wrapU == gfx::TextureWrapType::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D,
                                     GL_TEXTURE_WRAP_T,
                                     samplerState.wrapV == gfx::TextureWrapType::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT));
}

void Texture2D::bind(int32_t location, int32_t textureUnit) noexcept {
    using namespace platform;

    assert(gfx::MaxActiveTextureUnits > textureUnit);
    if (gfx::MaxActiveTextureUnits <= textureUnit) return;

    // Bind to the texture unit
    context.activeTextureUnit = static_cast<uint8_t>(textureUnit);
    context.texture[static_cast<size_t>(textureUnit)] = getTextureID();
    boundTextureUnit = textureUnit;

    // Update the sampler state if it was changed after resource creation
    if (samplerStateDirty) {
        updateSamplerConfiguration();
    }

    // Link the bound texture unit with the requested sampler in the active shader
    glUniform1i(location, textureUnit);
    boundLocation = location;
}

void Texture2D::unbind() noexcept {
    using namespace platform;

    // Unlink the texture from the last used texture unit
    if (boundTextureUnit != -1) {
        context.activeTextureUnit = boundTextureUnit;
        context.texture[static_cast<size_t>(boundTextureUnit)] = 0;
        boundTextureUnit = -1;
    }

    // And clear the uniform value linking a sampler to the texture unit
    // Default back to GL_TEXTURE0
    if (boundLocation != -1) {
        glUniform1i(boundLocation, 0);
        boundLocation = -1;
    }
}

void Texture2D::upload(const void* pixelData, const Size& size_) noexcept {
    using namespace platform;

    if (!textureResource || size_ == Size{0, 0} || size_ != size) {
        // Create the texture object if we don't already have one
        if (!textureResource) {
            createObject();
        }

        size = size_;
        createStorage(pixelData);

    } else {
        // Upload to existing memory
        uploadSubRegion(pixelData, size, 0, 0);
    }
}

void Texture2D::uploadSubRegion(const void* pixelData, const Size& size_, uint16_t xOffset, uint16_t yOffset) noexcept {
    using namespace platform;

    assert(textureResource);
    assert(!samplerStateDirty);

    // Bind to TU 0 and upload
    context.activeTextureUnit = 0;
    context.texture[0] = getTextureID();
    MBGL_CHECK_ERROR(glTexSubImage2D(GL_TEXTURE_2D,
                                     0,
                                     xOffset,
                                     yOffset,
                                     size_.width,
                                     size_.height,
                                     Enum<gfx::TexturePixelType>::to(pixelFormat),
                                     Enum<gfx::TextureChannelDataType>::to(channelType),
                                     pixelData));
}

void Texture2D::upload() noexcept {
    if (image && image->valid()) {
        setFormat(gfx::TexturePixelType::RGBA, gfx::TextureChannelDataType::UnsignedByte);
        upload(&(*image).data[0], image->size);
        image.reset();
    }
}

} // namespace gl
} // namespace mbgl
