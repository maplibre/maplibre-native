#include <functional>
#include <mln/platform/settings.hpp>
#include <mln/util/platform.hpp>

namespace mln {
namespace util {
std::function<void()> makeThreadPrioritySetter(std::string threadType_) {
    return [threadType = std::move(threadType_)] {
        auto& settings = platform::Settings::getInstance();
        auto value = settings.get(threadType);
        if (auto* priority = value.getDouble()) {
            platform::setCurrentThreadPriority(*priority);
        } else {
            platform::makeThreadLowPriority();
        }
    };
}
} // namespace util
} // namespace mln
