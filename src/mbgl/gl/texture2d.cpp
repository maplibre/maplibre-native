#include <mbgl/gl/texture2d.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/enum.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/util/instrumentation.hpp>

namespace mbgl {
namespace gl {

Texture2D::Texture2D(gl::Context& context_)
    : context(context_) {}

Texture2D::~Texture2D() {}

Texture2D& Texture2D::setSamplerConfiguration(const SamplerState& samplerState_) noexcept {
    samplerState = samplerState_;
    samplerStateDirty = texture != nullptr;
    return *this;
}

Texture2D& Texture2D::setFormat(gfx::TexturePixelType pixelFormat_, gfx::TextureChannelDataType channelType_) noexcept {
    if (pixelFormat_ == pixelFormat && channelType_ == channelType) {
        return *this;
    }

    assert(!texture);
    pixelFormat = pixelFormat_;
    channelType = channelType_;
    storageDirty = true;
    return *this;
}

Texture2D& Texture2D::setSize(mbgl::Size size_) noexcept {
    size = size_;
    storageDirty = true;
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

void Texture2D::allocateTexture() noexcept {
    MLN_TRACE_FUNC();

    // Create a new texture object
    auto obj = context.createUniqueTexture(size, pixelFormat, channelType);
    texture = std::make_unique<UniqueTexture>(std::move(obj));
}

void Texture2D::updateTextureData(const void* data) noexcept {
    MLN_TRACE_FUNC();
    if (data) {
        uploadSubRegion(data, size, 0, 0);
    }

    storageDirty = false;
    updateSamplerConfiguration();
}

void Texture2D::create() noexcept {
    allocateTexture();
    if (storageDirty) {
        updateTextureData();
    }
}

platform::GLuint Texture2D::getTextureID() const noexcept {
    return texture ? *texture : 0;
}

void Texture2D::updateSamplerConfiguration() noexcept {
    using namespace platform;
    samplerStateDirty = false;

    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D,
                                     GL_TEXTURE_MIN_FILTER,
                                     samplerState.filter == gfx::TextureFilterType::Nearest ? GL_NEAREST : GL_LINEAR));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D,
                                     GL_TEXTURE_MAG_FILTER,
                                     samplerState.filter == gfx::TextureFilterType::Nearest ? GL_NEAREST : GL_LINEAR));
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
    if (!texture || storageDirty || size_ == Size{0, 0} || size_ != size) {
        size = size_;

        // Create the texture object if we don't already have one or if storage is dirty
        allocateTexture();
    }
    updateTextureData(pixelData);
}

void Texture2D::uploadSubRegion(const void* pixelData, const Size& size_, uint16_t xOffset, uint16_t yOffset) noexcept {
    using namespace platform;

    assert(texture);

    // update sampler configuration if needed
    if (samplerStateDirty) {
        updateSamplerConfiguration();
    }

    // Bind to TU 0 and upload
    context.activeTextureUnit = 0;
    context.texture[0] = getTextureID();
    context.pixelStoreUnpack = {1};
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
        upload(image->data.get(), image->size);
        image.reset();
    }
}

} // namespace gl
} // namespace mbgl
