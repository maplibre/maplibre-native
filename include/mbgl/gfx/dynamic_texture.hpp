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
class TextureHandle;

class DynamicTexture {
public:
    DynamicTexture(Context& context, Size size);
    ~DynamicTexture() = default;

    const Texture2DPtr& getTextureAtlas();

    std::optional<TextureHandle> addImage(const AlphaImage& image, int32_t id = -1);
    void removeTexture(const TextureHandle& texHandle);

private:
    Texture2DPtr textureAtlas;
    mapbox::ShelfPack shelfPack;
    std::mutex mutex;
};

class TextureHandle {
public:
    TextureHandle(mapbox::Bin* bin_)
        : bin(bin_) {};
    ~TextureHandle() = default;

    mapbox::Bin* getBin() const { return bin; }

private:
    mapbox::Bin* bin;
};

} // namespace gfx
} // namespace mbgl
