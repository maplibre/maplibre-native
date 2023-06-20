#include "timer_impl.hpp"

#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/timer.hpp>

#include <memory>

namespace mbgl {
namespace util {

Timer::Impl::Impl() {
    timer.setTimerType(Qt::PreciseTimer);
    connect(&timer, &QTimer::timeout, this, &Timer::Impl::timerFired);
}

void Timer::Impl::start(uint64_t timeout, uint64_t repeat_, std::function<void()>&& cb) {
    repeat = repeat_;
    callback = std::move(cb);

    timer.setSingleShot(true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    timer.start(static_cast<std::chrono::milliseconds>(timeout));
#else
    timer.start(static_cast<int>(timeout));
#endif
}

void Timer::Impl::stop() {
    timer.stop();
}

void Timer::Impl::timerFired() {
    if (repeat) {
        timer.setSingleShot(false);
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
        timer.start(static_cast<std::chrono::milliseconds>(repeat));
#else
        timer.start(static_cast<int>(repeat));
#endif
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
