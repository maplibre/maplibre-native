#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>
#include <optional>

namespace mbgl {
namespace util {

struct IOException : std::runtime_error {
    IOException(int err, const std::string& msg);
    const int code = 0;
};

void write_file(const std::string& filename, const std::string& data);
std::string read_file(const std::string& filename);

std::optional<std::string> readFile(const std::string& filename,
                                    const std::optional<std::pair<uint64_t, uint64_t>>& dataRange = std::nullopt);
void deleteFile(const std::string& filename);
void copyFile(const std::string& destination, const std::string& source);

} // namespace util
} // namespace mbgl
