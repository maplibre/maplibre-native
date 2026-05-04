#pragma once

#include <mbgl/gfx/types.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/rect.hpp>

#include <mapbox/shelf-pack.hpp>

#include <optional>
#include <mutex>

namespace mbgl {
namespace gfx {

class Context;
class Texture2D;
using Texture2DPtr = std::shared_ptr<Texture2D>;

class TextureHandle {
public:
    TextureHandle(const mapbox::Bin& bin)
        : id(bin.id),
          rectangle(bin.x, bin.y, bin.w, bin.h),
          needsUpload(bin.refcount() == 1) {};
    ~TextureHandle() = default;

    int32_t getId() const { return id; }
    const Rect<uint16_t>& getRectangle() const { return rectangle; }
    bool isUploadNeeded() const { return needsUpload; }

    bool operator==(const TextureHandle& other) const { return (id == other.id); }

    struct Hasher {
        size_t operator()(const TextureHandle& texHandle) const { return texHandle.id; }
    };

private:
    int32_t id = 0;
    Rect<uint16_t> rectangle;
    bool needsUpload = false;

    friend class DynamicTexture;
};

class DynamicTexture {
public:
    DynamicTexture(Context& context, Size size, TexturePixelType pixelType);
    virtual ~DynamicTexture() = default;

    const Texture2DPtr& getTexture() const;
    TexturePixelType getPixelFormat() const;
    bool isEmpty() const;

    std::optional<TextureHandle> reserveSize(const Size& size, int32_t uniqueId);

    template <typename Image>
    std::optional<TextureHandle> addImage(const Image& image, int32_t uniqueId = -1) {
        return addImage(image.data ? image.data.get() : nullptr, image.size, uniqueId);
    }
    std::optional<TextureHandle> addImage(const uint8_t* pixelData, const Size& imageSize, int32_t uniqueId = -1);

    virtual void uploadImage(const uint8_t* pixelData, TextureHandle& texHandle);
    virtual void uploadDeferredImages() {};
    virtual bool removeTexture(const TextureHandle& texHandle);

protected:
    mapbox::ShelfPack shelfPack;
    Texture2DPtr texture;
    int numTextures = 0;
    std::mutex mutex;
};

} // namespace gfx
} // namespace mbgl
