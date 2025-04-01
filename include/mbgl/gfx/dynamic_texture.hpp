#pragma once

#include <mbgl/gfx/types.hpp>
#include <mbgl/util/image.hpp>

#include <mapbox/shelf-pack.hpp>

namespace mbgl {

namespace gfx {

class Context;
class Texture2D;
using Texture2DPtr = std::shared_ptr<gfx::Texture2D>;
using ImagesToUpload = std::unordered_map<mapbox::Bin*, std::unique_ptr<uint8_t[]>>;

class TextureHandle {
public:
    TextureHandle(mapbox::Bin* bin_)
        : bin(bin_) {};
    ~TextureHandle() = default;

    mapbox::Bin* getBin() const { return bin; }

private:
    mapbox::Bin* bin;
};

class DynamicTexture {
public:
    DynamicTexture(Context& context, Size size, TexturePixelType pixelType);
    ~DynamicTexture() = default;

    const Texture2DPtr& getTexture() const;
    TexturePixelType getPixelFormat() const;

    template <typename Image>
    std::optional<TextureHandle> addImage(const Image& image, int32_t id = -1) {
        return addImage(image.data ? image.data.get() : nullptr, image.size, id);
    }
    std::optional<TextureHandle> addImage(const uint8_t* pixelData, const Size& imageSize, int32_t id = -1);
    void uploadDeferredImages();
    void removeTexture(const TextureHandle& texHandle);

private:
    Texture2DPtr texture;
    mapbox::ShelfPack shelfPack;
    ImagesToUpload imagesToUpload;
};

#define MLN_DEFER_UPLOAD_ON_RENDER_THREAD (MLN_RENDER_BACKEND_OPENGL || MLN_RENDER_BACKEND_VULKAN)

} // namespace gfx
} // namespace mbgl
