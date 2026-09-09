#pragma once

#include <string>

#if defined(_WIN32)
#include <filesystem>
#endif

namespace mln {
namespace util {
bool is_absolute_path(std::string path);

#if defined(_WIN32)
/// The narrow std::filesystem::path constructor decodes with the active code
/// page on Windows, so the UTF-8 bytes go through the char8_t constructor.
inline std::filesystem::path pathFromUTF8(const std::string& path) {
    return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(path.data()), path.size()));
}
#else
inline const std::string& pathFromUTF8(const std::string& path) {
    return path;
}
#endif
} // namespace util
} // namespace mln
