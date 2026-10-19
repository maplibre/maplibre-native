#include <mln/util/image.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/premultiply.hpp>

#include <webp/types.h>
#include <webp/decode.h>

namespace mln {

PremultipliedImage decodeWEBP(const uint8_t* data, size_t size) {
    int32_t width, height;
    if (!WebPGetInfo(data, size, &width, &height)) {
        Log::Warning(Event::Image, "Failed to decode WebP image header!");
        return {};
    }

    auto img = UnassociatedImage({static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
    if (!WebPDecodeRGBAInto(data, size, img.data.get(), img.bytes(), static_cast<int32_t>(img.stride()))) {
        Log::Warning(Event::Image, "Failed to decode WebP image contents!");
        return {};
    }

    return util::premultiply(std::move(img));
}

} // namespace mln
