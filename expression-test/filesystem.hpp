#pragma once

#ifdef WIN32
#define GHC_WIN_DISABLE_WSTRING_STORAGE_TYPE
#endif

#include <ghc/filesystem.hpp>

namespace mbgl {

namespace filesystem = ghc::filesystem;

} // namespace mbgl
