#include <mbgl/util/image.hpp>
#include <webp/types.h>
#include <webp/decode.h>

namespace mbgl {

PremultipliedImage decodeWEBP(const uint8_t* data, size_t size) {
    int32_t width, height;
    auto bytes = WebPDecodeRGBA(data, size, &width, &height);
    if (!bytes) {
        return {};
    }

    const auto rawSize = 4 * width * height; // RGBA = 4
    auto img = PremultipliedImage({static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, bytes, rawSize);
    WebPFree(bytes);
    return img;
}

} // namespace mbgl
