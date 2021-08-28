#pragma once

#include <string>

namespace mbgl {
namespace util {

enum CompressionFormat { ZLIB = 15, GZIP = 15 + 16, DEFLATE = -15, DETECT = 15 + 32 };

std::string compress(const std::string& raw, int windowBits = CompressionFormat::ZLIB);
std::string decompress(const std::string& raw, int windowBits = CompressionFormat::DETECT);

} // namespace util
} // namespace mbgl
