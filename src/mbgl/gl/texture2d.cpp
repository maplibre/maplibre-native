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
    pixelFormat = pixelFormat_;
    channelType = channelType_;
    return *this;
}

Texture2D& Texture2D::setSize(mbgl::Size size_) noexcept {
    size = size_;
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
        default:
            return 0;
    }
}

void Texture2D::create(const std::vector<uint8_t>& pixelData, gfx::UploadPass& uploadPass) noexcept {
    assert(!textureResource);
    textureResource = uploadPass.createTextureResource(size, pixelData.data(), pixelFormat, channelType);
}

void Texture2D::create() noexcept {
    assert(!textureResource);
    textureResource = context.createTextureResource(size, pixelFormat, channelType);
}

platform::GLuint Texture2D::getTextureID() const noexcept {
    return static_cast<gl::TextureResource&>(*textureResource).texture;
}

void Texture2D::bind(int32_t location, int32_t textureUnit) noexcept {
    using namespace platform;

    assert(gfx::MaxActiveTextureUnits > textureUnit);
    if (gfx::MaxActiveTextureUnits > textureUnit) return;

    context.activeTextureUnit = static_cast<uint8_t>(textureUnit);
    context.texture[static_cast<size_t>(textureUnit)] = getTextureID();

    boundTextureUnit = textureUnit;

    // Update the sampler state if it was changed after resource creation
    if (samplerStateDirty) {
        samplerStateDirty = false;

        MBGL_CHECK_ERROR(
            glTexParameteri(GL_TEXTURE_2D,
                            GL_TEXTURE_MIN_FILTER,
                            samplerState.minification == gfx::TextureFilterType::Nearest ? GL_NEAREST : GL_LINEAR));
        MBGL_CHECK_ERROR(
            glTexParameteri(GL_TEXTURE_2D,
                            GL_TEXTURE_MAG_FILTER,
                            samplerState.magnification == gfx::TextureFilterType::Nearest ? GL_NEAREST : GL_LINEAR));
        MBGL_CHECK_ERROR(
            glTexParameteri(GL_TEXTURE_2D,
                            GL_TEXTURE_WRAP_S,
                            samplerState.wrapU == gfx::TextureWrapType::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT));
        MBGL_CHECK_ERROR(
            glTexParameteri(GL_TEXTURE_2D,
                            GL_TEXTURE_WRAP_T,
                            samplerState.wrapV == gfx::TextureWrapType::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT));
    }

    // Link the bound texture unit with the requested sampler in the active shader
    glUniform1i(location, textureUnit);
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

void Texture2D::upload(const PremultipliedImage& image, gfx::UploadPass& uploadPass) const noexcept {
    assert(textureResource);
    assert(image.size == size);
    if (image.size != size) {
        return;
    }

    assert(PremultipliedImage::channels == numChannels());
    if (PremultipliedImage::channels != numChannels()) {
        return;
    }

    // note: images are always unsigned bytes
    assert(PremultipliedImage::channels == getPixelStride());
    if (PremultipliedImage::channels != getPixelStride()) {
        return;
    }

    uploadPass.updateTextureResource(*textureResource, size, &image.data[0], pixelFormat, channelType);
}

} // namespace gl
} // namespace mbgl
