#include <mbgl/util/compression.hpp>

#if defined(__QT__) && (defined(_WIN32) || defined(__EMSCRIPTEN__))
#include <QtZlib/zlib.h>
#else
#include <zlib.h>
#endif

#include <cstdio>
#include <cstring>
#include <stdexcept>

// Check zlib library version.
[[maybe_unused]] const static bool zlibVersionCheck = []() {
    const char *const version = zlibVersion();
    if (version[0] != ZLIB_VERSION[0]) {
        char message[96];
        snprintf(
            message, 96, "zlib version mismatch: headers report %s, but library reports %s", ZLIB_VERSION, version);
        throw std::runtime_error(message);
    }

    return true;
}();

namespace mbgl {
namespace util {

// Needed when using a zlib compiled with -DZ_PREFIX
// because it will mess with this function name and
// cause a link error.
#undef compress

bool is_compressed(const std::string &v) {
    if (v.size() > 2) {
        const auto byte0 = static_cast<uint8_t>(v[0]);
        const auto byte1 = static_cast<uint8_t>(v[1]);
        if (byte0 == 0x1f && byte1 == 0x8b) {
            // gzip (rfc1952)
            return true;
        } else if (byte0 == 0x78) {
            // zlib (rfc1950)
            switch (byte1) {
                case 0x01: // 78 01 - No Compression/low
                case 0x5E: // 78 5E - Fast Compression
                case 0x9C: // 78 9C - Default Compression
                case 0xDA: // 78 DA - Best Compression
                    return true;
                default:
                    return false;
            }
        }
    }
    return false;
}

std::string compress(const std::string &raw, int windowBits) {
    z_stream deflate_stream;
    memset(&deflate_stream, 0, sizeof(deflate_stream));

    // TODO: reuse z_streams
    if (deflateInit2(&deflate_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw std::runtime_error("failed to initialize deflate");
    }

    deflate_stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(raw.data()));
    deflate_stream.avail_in = uInt(raw.size());

    std::string result;
    char out[16384];

    int code;
    do {
        deflate_stream.next_out = reinterpret_cast<Bytef *>(out);
        deflate_stream.avail_out = sizeof(out);
        code = deflate(&deflate_stream, Z_FINISH);
        if (result.size() < deflate_stream.total_out) {
            // append the block to the output string
            result.append(out, deflate_stream.total_out - result.size());
        }
    } while (code == Z_OK);

    deflateEnd(&deflate_stream);

    if (code != Z_STREAM_END) {
        throw std::runtime_error(deflate_stream.msg);
    }

    return result;
}

std::string decompress(const std::string &raw, int windowBits) {
    z_stream inflate_stream;
    memset(&inflate_stream, 0, sizeof(inflate_stream));

    // TODO: reuse z_streams
    if (inflateInit2(&inflate_stream, windowBits) != Z_OK) {
        throw std::runtime_error("failed to initialize inflate");
    }

    inflate_stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(raw.data()));
    inflate_stream.avail_in = uInt(raw.size());

    std::string result;
    char out[16384];

    int code;
    do {
        inflate_stream.next_out = reinterpret_cast<Bytef *>(out);
        inflate_stream.avail_out = sizeof(out);
        code = inflate(&inflate_stream, 0);
        if (result.size() < inflate_stream.total_out) {
            result.append(out, inflate_stream.total_out - result.size());
        }
    } while (code == Z_OK);

    inflateEnd(&inflate_stream);

    if (code != Z_STREAM_END) {
        throw std::runtime_error(inflate_stream.msg ? inflate_stream.msg : "decompression error");
    }

    return result;
}

std::uint32_t crc32(const void *raw, size_t size) noexcept {
    auto hash = ::crc32(0L, Z_NULL, 0);
    if (raw) {
        const auto *p = static_cast<const Bytef *>(raw);
        while (size > 0) {
            const auto blockSize = static_cast<uInt>(size);
            hash = ::crc32(hash, p, blockSize);
            p += blockSize;
            size -= blockSize;
        }
    }
    return static_cast<std::uint32_t>(hash);
}

} // namespace util
} // namespace mbgl
