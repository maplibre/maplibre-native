#include <mln/util/platform.hpp>
#include <mln/platform/thread.hpp>

#include <string>

namespace mln {
namespace platform {

std::string getCurrentThreadName() {
    return "unknown";
}

void setCurrentThreadName(const std::string&) {}

void makeThreadLowPriority() {}

void setCurrentThreadPriority(double) {}

void attachThread() {}

void detachThread() {}

} // namespace platform
} // namespace mln
