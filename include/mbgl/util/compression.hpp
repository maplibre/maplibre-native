#pragma once

#include <cstdint>
#include <string>

namespace mbgl {
namespace util {

enum CompressionFormat {
    ZLIB = 15,
    GZIP = 15 + 16,
    DEFLATE = -15,
    DETECT = 15 + 32
};

bool is_compressed(const std::string&);
std::string compress(const std::string& raw, int windowBits = CompressionFormat::ZLIB);
std::string decompress(const std::string& raw, int windowBits = CompressionFormat::DETECT);

std::uint32_t crc32(const void* raw, size_t size) noexcept;

} // namespace util
} // namespace mbgl
