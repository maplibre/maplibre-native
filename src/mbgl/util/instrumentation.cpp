#include <mbgl/util/instrumentation.hpp>

namespace mbgl::instrumentation {

void setThreadName(const std::string &name) {
    if constexpr (tracyEnabled) {
        tracy::SetThreadName(name.c_str());
    }
}

}; // namespace mbgl::instrumentation
