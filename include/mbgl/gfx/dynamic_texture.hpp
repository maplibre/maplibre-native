#pragma once

#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/util/image.hpp>

#include <mapbox/shelf-pack.hpp>

namespace mbgl {

namespace gfx {

class Context;
class Texture2D;
using Texture2DPtr = std::shared_ptr<gfx::Texture2D>;

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

    const Texture2DPtr& getTextureAtlas();

    template <typename Image>
    std::optional<TextureHandle> addImage(const Image& image, int32_t id = -1) {
        return addImage(image.data ? image.data.get() : nullptr, image.size, id);
    }
    std::optional<TextureHandle> addImage(const void* pixelData, const Size& imageSize, int32_t id = -1);
    
    void removeTexture(const TextureHandle& texHandle);

private:
    Texture2DPtr textureAtlas;
    mapbox::ShelfPack shelfPack;
};

} // namespace gfx
} // namespace mbgl
