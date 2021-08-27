#pragma once

#include <string>

namespace mbgl {
namespace util {

enum CompressionFormat { ZLIB = 15, GZIP = 31, DEFLATE = -15 };

std::string compress(const std::string& raw, int windowBits = CompressionFormat::ZLIB);
std::string decompress(const std::string& raw);

} // namespace util
} // namespace mbgl
