#pragma once

#include <string>

// libc++ filesystem requires macOS 10.15, iOS/tvOS 13, or watchOS 6.
// Check the deployment target, not the SDK version used to compile.
#if defined(__APPLE__) &&                                                                                            \
    ((defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) &&                                                      \
      __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101500) ||                                                     \
     (defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) &&                                                     \
      __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ < 130000) ||                                                    \
     (defined(__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__) && __ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__ < 130000) || \
     (defined(__ENVIRONMENT_WATCH_OS_VERSION_MIN_REQUIRED__) &&                                                      \
      __ENVIRONMENT_WATCH_OS_VERSION_MIN_REQUIRED__ < 60000))
#define MLN_HAS_STD_FILESYSTEM 0
#else
#define MLN_HAS_STD_FILESYSTEM 1
#endif

#if MLN_HAS_STD_FILESYSTEM
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
