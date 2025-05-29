#include <mbgl/util/monotonic_timer.hpp>

#include <cassert>
#include <chrono>

namespace mbgl {
namespace util {

// Prefer high resolution timer if it is monotonic
template <typename T>
static T sample()
    requires(std::chrono::high_resolution_clock::is_steady)
{
    return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

template <typename T>
static T sample()
    requires(!std::chrono::high_resolution_clock::is_steady)
{
    return std::chrono::duration_cast<T>(std::chrono::steady_clock::now().time_since_epoch());
}

std::chrono::duration<double> MonotonicTimer::now() {
    return sample<std::chrono::duration<double>>();
}

} // namespace util
} // namespace mbgl
