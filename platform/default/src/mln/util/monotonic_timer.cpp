#include <mln/util/monotonic_timer.hpp>

#include <cassert>
#include <chrono>
#include <type_traits>

namespace mln {
namespace util {

// Prefer high resolution timer if it is monotonic
namespace {
using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
                                 std::chrono::high_resolution_clock,
                                 std::chrono::steady_clock>;

template <typename T>
T sample() {
    return std::chrono::duration_cast<T>(Clock::now().time_since_epoch());
}
} // namespace

std::chrono::duration<double> MonotonicTimer::now() {
    return sample<std::chrono::duration<double>>();
}

} // namespace util
} // namespace mln
