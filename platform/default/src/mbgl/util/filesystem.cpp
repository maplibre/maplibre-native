#include <mbgl/util/filesystem.hpp>

#if defined(USE_STD_FILESYSTEM)
#include <filesystem>

bool mbgl::util::is_absolute_path(std::string path) {
    return std::filesystem::path(path).is_absolute();
}
#else
bool mbgl::util::is_absolute_path(std::string path) {
    return path.at(0) == '/';
}
#endif
