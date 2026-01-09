#pragma once

#include <fstream>
#include <sstream>
#include <string>

#include "mapbox/util/expected.hpp"

namespace mapbox {
namespace base {
namespace io {

using ErrorType = std::string;

inline expected<std::string, ErrorType> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.good()) {
        return make_unexpected(std::string("Failed to read file '") + filename + std::string("'"));
    }

    std::stringstream data;
    data << file.rdbuf();
    return expected<std::string, ErrorType>(data.str());
}

inline expected<void, ErrorType> writeFile(const std::string& filename, const std::string& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.good()) {
        return make_unexpected(std::string("Failed to write file '") + filename + std::string("'"));
    }

    file << data;

    return expected<void, ErrorType>();
}

inline expected<void, ErrorType> deleteFile(const std::string& filename) {
    const int ret = std::remove(filename.c_str());
    if (ret != 0) {
        return make_unexpected(std::string("Failed to delete file '") + filename + std::string("'"));
    }

    return expected<void, ErrorType>();
}

inline expected<void, ErrorType> copyFile(const std::string& sourcePath, const std::string& destinationPath) {
    auto contents = readFile(sourcePath);
    if (!contents) {
        return nonstd::make_unexpected(contents.error());
    }

    return writeFile(destinationPath, *contents);
}

} // namespace io
} // namespace base
} // namespace mapbox
