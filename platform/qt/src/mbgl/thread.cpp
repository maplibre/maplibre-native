#include <mbgl/util/platform.hpp>
#include <mbgl/platform/thread.hpp>

#include <string>

namespace mbgl {
namespace platform {

std::string getCurrentThreadName() {
    return "unknown";
}

void setCurrentThreadName(const std::string&) {}

void makeThreadLowPriority() {}

void makeThreadHighPriority() {}

double getCurrentThreadPriority() { return 0;}

void setCurrentThreadPriority(double) {}

void attachThread() {}

void detachThread() {}

} // namespace platform
} // namespace mbgl
