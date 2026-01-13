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
    assert(!Scheduler::GetCurrent(false));
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

void RunLoop::waitForEmpty([[maybe_unused]] const SimpleIdentity tag) {
    while (true) {
        std::size_t remaining;
        {
            std::scoped_lock lock(mutex);
            remaining = defaultQueue.size() + highPriorityQueue.size();
        }

        if (remaining == 0) {
            return;
        }

        runOnce();
    }
}

} // namespace util
} // namespace mbgl
