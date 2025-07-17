#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/thread_local.hpp>
#include <mbgl/util/thread_pool.hpp>
#include <mbgl/util/run_loop.hpp>

namespace mbgl {

std::function<void()> Scheduler::bindOnce(std::function<void()> fn) {
    assert(fn);
    return [scheduler = makeWeakPtr(), scheduled = std::move(fn)]() mutable {
        if (!scheduled) return; // Repeated call.
        if (auto guard = scheduler.lock(); scheduler) {
            scheduler->schedule(std::move(scheduled));
        }
    };
}

namespace {

thread_local Scheduler* localScheduler;
} // namespace

void Scheduler::SetCurrent(Scheduler* scheduler) {
    localScheduler = scheduler;
}

Scheduler* Scheduler::GetCurrent(bool init) {
    if (!localScheduler && init) {
        static thread_local util::RunLoop runLoop;
        SetCurrent(&runLoop);
    }
    return localScheduler;
}

// static
std::shared_ptr<Scheduler> Scheduler::GetBackground() {
    static std::weak_ptr<Scheduler> weak;
    static std::mutex mtx;

    std::lock_guard<std::mutex> lock(mtx);
    std::shared_ptr<Scheduler> scheduler = weak.lock();

    if (!scheduler) {
        weak = scheduler = std::make_shared<ThreadPool>();
    }

    return scheduler;
}

// static
std::shared_ptr<Scheduler> Scheduler::GetSequenced() {
    constexpr std::size_t kSchedulersCount = 10;
    static std::vector<std::weak_ptr<Scheduler>> weaks(kSchedulersCount);
    static std::mutex mtx;
    static std::size_t lastUsedIndex = 0u;

    std::lock_guard lock(mtx);

    lastUsedIndex = (lastUsedIndex + 1) % kSchedulersCount;

    if (auto scheduler = weaks[lastUsedIndex].lock()) {
        return scheduler;
    } else {
        auto result = std::make_shared<SequencedScheduler>();
        weaks[lastUsedIndex] = result;
        return result;
    }
}

} // namespace mbgl
