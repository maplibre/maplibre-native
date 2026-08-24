#include <mln/util/filesystem.hpp>

#if defined(USE_STD_FILESYSTEM)
#include <filesystem>

bool mln::util::is_absolute_path(std::string path) {
    return std::filesystem::path(path).is_absolute();
}
#else
bool mln::util::is_absolute_path(std::string path) {
    return path.at(0) == '/';
}
#endif
