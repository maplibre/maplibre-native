#include <mbgl/webgpu/dynamic_texture.hpp>
#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/webgpu/context.hpp>

namespace mbgl {
namespace webgpu {

DynamicTexture::DynamicTexture(Context& context, Size size, gfx::TexturePixelType pixelType)
    : gfx::DynamicTexture(size, pixelType),
      context(context),
      size(size),
      pixelType(pixelType) {
    deferredCreation = true;
}

namespace {
size_t pixelStride(gfx::TexturePixelType pixelType) {
    switch (pixelType) {
        case gfx::TexturePixelType::Alpha:
        case gfx::TexturePixelType::Luminance:
            return 1;
        case gfx::TexturePixelType::RGBA:
            return 4;
        case gfx::TexturePixelType::Stencil:
        case gfx::TexturePixelType::Depth:
            return 4;
    }
    return 4;
}
} // namespace

void DynamicTexture::uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) {
    std::scoped_lock lock(mutex);
    const auto& rect = texHandle.getRectangle();
    const auto imageSize = Size(rect.w, rect.h);

    auto byteSize = imageSize.area() * pixelStride(pixelType);
    auto imageData = std::make_unique<uint8_t[]>(byteSize);
    std::copy(pixelData, pixelData + byteSize, imageData.get());
    imagesToUpload.emplace(texHandle, std::move(imageData));

    gfx::DynamicTexture::uploadImage(pixelData, texHandle);
}

void DynamicTexture::uploadDeferredImages(gfx::UploadPass&) {
    std::scoped_lock lock(mutex);
    if (deferredCreation) {
        texture = context.createTexture2D();
        texture->setSize(size);
        texture->setFormat(pixelType, gfx::TextureChannelDataType::UnsignedByte);
        texture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                          .wrapU = gfx::TextureWrapType::Clamp,
                                          .wrapV = gfx::TextureWrapType::Clamp});
        texture->create();
        deferredCreation = false;
    }
    for (const auto& pair : imagesToUpload) {
        const auto& rect = pair.first.getRectangle();
        texture->uploadSubRegion(pair.second.get(), Size(rect.w, rect.h), rect.x, rect.y);
    }
    imagesToUpload.clear();
}

bool DynamicTexture::removeTexture(const gfx::TextureHandle& texHandle) {
    if (gfx::DynamicTexture::removeTexture(texHandle)) {
        std::scoped_lock lock(mutex);
        imagesToUpload.erase(texHandle);
        return true;
    }
    return false;
}

} // namespace webgpu
} // namespace mbgl
