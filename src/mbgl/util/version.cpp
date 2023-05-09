#include <mbgl/util/version.hpp>

namespace mbgl {
namespace version {

#ifdef MBGL_VERSION_REV
const char* revision = MBGL_VERSION_REV;
#else
const char* revision = "NOT SET";
#endif

} // namespace version
} // namespace mbgl
