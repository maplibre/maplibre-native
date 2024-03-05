#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/async_task.hpp>
#include <mbgl/util/monotonic_timer.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/string.hpp>

#include <CoreFoundation/CoreFoundation.h>

namespace mbgl {
namespace util {

class RunLoop::Impl {
public:
    std::unique_ptr<AsyncTask> async;
};

RunLoop* RunLoop::Get() {
    assert(static_cast<RunLoop*>(Scheduler::GetCurrent()));
    return static_cast<RunLoop*>(Scheduler::GetCurrent());
}

RunLoop::RunLoop(Type)
  : impl(std::make_unique<Impl>()) {
    assert(!Scheduler::GetCurrent());
    Scheduler::SetCurrent(this);
    impl->async = std::make_unique<AsyncTask>(std::bind(&RunLoop::process, this));
}

RunLoop::~RunLoop() {
    Scheduler::SetCurrent(nullptr);
}

void RunLoop::wake() {
    impl->async->send();
}

void RunLoop::run() {
    CFRunLoopRun();
}

void RunLoop::runOnce() {
    wake();
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
}

void RunLoop::stop() {
    invoke([&] { CFRunLoopStop(CFRunLoopGetCurrent()); });
}

std::size_t RunLoop::waitForEmpty(Milliseconds timeout) {
    const auto startTime = mbgl::util::MonotonicTimer::now();
    while (true) {
        std::size_t remaining;
        {
            std::lock_guard<std::mutex> lock(mutex);
            remaining = defaultQueue.size() + highPriorityQueue.size();
        }

        const auto elapsed = mbgl::util::MonotonicTimer::now() - startTime;
        const auto elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        if (remaining == 0 || (Milliseconds::zero() < timeout && timeout <= elapsedMillis)) {
            return remaining;
        }

        runOnce();
    }
}

} // namespace util
} // namespace mbgl
