#include <mbgl/util/image.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/premultiply.hpp>

namespace mbgl {

PremultipliedImage decodePNG(const uint8_t*, size_t);
PremultipliedImage decodeJPEG(const uint8_t*, size_t);
PremultipliedImage decodeWEBP(const uint8_t*, size_t);

PremultipliedImage decodeImage(const std::string& string) {
    const auto* data = reinterpret_cast<const uint8_t*>(string.data());
    const size_t size = string.size();

    const auto readUInt = [](const uint8_t* imageData, ptrdiff_t offset) -> uint32_t {
        const auto ptr = imageData + offset;
        return (static_cast<uint32_t>(ptr[0]) << 24) | (static_cast<uint32_t>(ptr[1]) << 16) |
               (static_cast<uint32_t>(ptr[2]) << 8) | static_cast<uint32_t>(ptr[3]);
    };

    if (size >= 16) {
        uint32_t magic1 = readUInt(data, 0x0);
        uint32_t magic2 = readUInt(data, 0x8);
        // RIFF <xxxx = file size> WEBP
        if (magic1 == 0x52494646 && magic2 == 0x57454250) {
            return decodeWEBP(data, size);
        }
    }

    if (size >= 4) {
        uint32_t magic = readUInt(data, 0x0);
        if (magic == 0x89504E47U) {
            return decodePNG(data, size);
        }
    }

    if (size >= 2) {
        uint16_t magic = ((data[0] << 8) | data[1]) & 0xffff;
        if (magic == 0xFFD8) {
            return decodeJPEG(data, size);
        }
    }

    throw std::runtime_error("unsupported image type");
}

} // namespace mbgl
