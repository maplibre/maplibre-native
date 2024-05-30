#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/thread_local.hpp>
#include <mbgl/util/thread_pool.hpp>

namespace mbgl {

std::function<void()> Scheduler::bindOnce(std::function<void()> fn) {
    assert(fn);
    return [scheduler = makeWeakPtr(), scheduled = std::move(fn)]() mutable {
        if (!scheduled) return; // Repeated call.
        auto schedulerGuard = scheduler.lock();
        if (scheduler) scheduler->schedule(std::move(scheduled));
    };
}

static auto& current() {
    static util::ThreadLocal<Scheduler> scheduler;
    return scheduler;
};

void Scheduler::SetCurrent(Scheduler* scheduler) {
    current().set(scheduler);
}

Scheduler* Scheduler::GetCurrent() {
    return current().get();
}

// static
std::shared_ptr<Scheduler> Scheduler::GetBackground() {
    static std::shared_ptr<Scheduler> scheduler;
    static std::mutex mtx;

    std::lock_guard<std::mutex> lock(mtx);

    if (!scheduler) {
        scheduler = std::make_shared<ThreadPool>();
    }

    return scheduler;
}

// static
std::shared_ptr<Scheduler> Scheduler::GetSequenced() {
    const std::size_t kSchedulersCount = 10;
    static std::vector<std::shared_ptr<Scheduler>> schedulers(kSchedulersCount);
    static std::mutex mtx;
    static std::size_t lastUsedIndex = 0u;

    std::lock_guard<std::mutex> lock(mtx);

    const auto index = (++lastUsedIndex) % kSchedulersCount;
    if (!schedulers[index]) {
        schedulers[index] = std::make_shared<SequencedScheduler>();
    }
    return schedulers[index];
}

} // namespace mbgl
