#include "timer_impl.hpp"

#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/timer.hpp>

#include <algorithm>
#include <limits>
#include <memory>

namespace mbgl {
namespace util {

namespace {
// QTimer stores its millisecond interval in an int, so any value above
// INT_MAX (~24.85 days) is clamped by Qt and produces a runtime warning:
//   "QTimer::start: interval exceeds maximum allowed interval, it will be
//    clamped to INT_MAX ms (about 24 days)."
// MapLibre core can legitimately schedule timers far in the future, e.g.
// when a tile arrives with a long-lived `Cache-Control: max-age=31536000`
// (one year) header and the cache expiry timer is armed from it. Cap the
// value here so Qt stays silent; anything beyond INT_MAX ms is effectively
// "never fires" for any practical map session anyway.
constexpr uint64_t kMaxIntervalMs = static_cast<uint64_t>(std::numeric_limits<int>::max());

std::chrono::milliseconds clampInterval(uint64_t value) {
    return std::chrono::milliseconds(std::min(value, kMaxIntervalMs));
}
} // namespace

Timer::Impl::Impl() {
    timer.setTimerType(Qt::PreciseTimer);
    connect(&timer, &QTimer::timeout, this, &Timer::Impl::timerFired);
}

void Timer::Impl::start(uint64_t timeout, uint64_t repeat_, std::function<void()>&& cb) {
    repeat = repeat_;
    callback = std::move(cb);

    timer.setSingleShot(true);
    timer.start(clampInterval(timeout));
}

void Timer::Impl::stop() {
    timer.stop();
}

void Timer::Impl::timerFired() {
    if (repeat) {
        timer.setSingleShot(false);
        timer.start(clampInterval(repeat));
    }

    callback();
}

Timer::Timer()
    : impl(std::make_unique<Impl>()) {}

Timer::~Timer() = default;

void Timer::start(Duration timeout, Duration repeat, std::function<void()>&& cb) {
    impl->start(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
                std::chrono::duration_cast<std::chrono::milliseconds>(repeat).count(),
                std::move(cb));
}

void Timer::stop() {
    impl->stop();
}

} // namespace util
} // namespace mbgl
