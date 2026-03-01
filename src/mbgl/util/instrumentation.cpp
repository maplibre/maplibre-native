#include <mbgl/util/instrumentation.hpp>

namespace mbgl::instrumentation {

void setThreadName([[maybe_unused]] const std::string &name) {
#ifdef MLN_TRACY_ENABLE
    tracy::SetThreadName(name.c_str());
#endif
}

}; // namespace mbgl::instrumentation
