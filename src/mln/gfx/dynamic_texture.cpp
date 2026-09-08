#include <mln/gfx/dynamic_texture.hpp>
#include <mln/gfx/texture2d.hpp>
#include <mln/gfx/context.hpp>

namespace mln {
namespace gfx {

DynamicTexture::DynamicTexture(Size size, TexturePixelType pixelType)
    : pixelFormat(pixelType) {
    mapbox::ShelfPack::ShelfPackOptions options;
    options.autoResize = false;
    shelfPack = mapbox::ShelfPack(size.width, size.height, options);
}

DynamicTexture::DynamicTexture(Context& context, Size size, TexturePixelType pixelType)
    : DynamicTexture(size, pixelType) {
    texture = context.createTexture2D();
    texture->setSize(size);
    texture->setFormat(pixelType, gfx::TextureChannelDataType::UnsignedByte);
    texture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                      .wrapU = gfx::TextureWrapType::Clamp,
                                      .wrapV = gfx::TextureWrapType::Clamp});
}

const gfx::Texture2DPtr& DynamicTexture::getTexture() const {
    assert(texture);
    return texture;
}

TexturePixelType DynamicTexture::getPixelFormat() const {
    return pixelFormat;
}

bool DynamicTexture::isEmpty() const {
    return (numTextures == 0);
}

int32_t DynamicTexture::getVersion(int32_t uniqueId) const {
    if (versionMap.contains(uniqueId)) {
        return versionMap.at(uniqueId);
    }
    return -1;
}

std::optional<TextureHandle> DynamicTexture::reserveSize(const Size& size, int32_t uniqueId) {
    std::scoped_lock lock(mutex);
    mapbox::Bin* bin = shelfPack.packOne(uniqueId, size.width, size.height);
    if (!bin) {
        return std::nullopt;
    }
    if (bin->refcount() == 1) {
        numTextures++;
    }
    return TextureHandle(*bin);
}

std::optional<TextureHandle> DynamicTexture::addImage(const uint8_t* pixelData,
                                                      const Size& imageSize,
                                                      int32_t uniqueId) {
    auto texHandle = reserveSize(imageSize, uniqueId);
    if (texHandle && texHandle->isUploadNeeded()) {
        uploadImage(pixelData, *texHandle);
    }
    return texHandle;
}

void DynamicTexture::uploadImage(const uint8_t* /*pixelData*/, gfx::TextureHandle& texHandle) {
    texHandle.needsUpload = false;
    int32_t version = 0;
    if (versionMap.contains(texHandle.getId())) {
        version = versionMap[texHandle.getId()] + 1;
    }
    versionMap[texHandle.getId()] = version;
}

bool DynamicTexture::removeTexture(const TextureHandle& texHandle) {
    std::scoped_lock lock(mutex);
    auto* bin = shelfPack.getBin(texHandle.getId());
    if (!bin) {
        return false;
    }
    auto refcount = shelfPack.unref(*bin);
    if (refcount == 0) {
        numTextures--;
        versionMap.erase(texHandle.getId());
        return true;
    }
    return false;
}

} // namespace gfx
} // namespace mln
