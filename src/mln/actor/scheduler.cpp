#include <mln/actor/scheduler.hpp>
#include <mln/util/thread_local.hpp>
#include <mln/util/thread_pool.hpp>
#include <mln/util/run_loop.hpp>

#include <atomic>
#include <cassert>
#include <future>
#include <mutex>
#include <vector>

namespace mln {

// Owned by the scheduler; a WeakScheduler holds it only while it schedules work.
struct WeakScheduler::Anchor {
    explicit Anchor(Scheduler& scheduler_)
        : scheduler(scheduler_) {}

    ~Anchor() { released.set_value(); }

    Scheduler& scheduler;
    std::atomic<bool> expiring{false};
    std::promise<void> released;
};

bool WeakScheduler::schedule(std::function<void()>&& fn) const {
    const auto locked = anchor.lock();
    if (!locked || locked->expiring) {
        return false;
    }
    locked->scheduler.schedule(std::move(fn));
    return true;
}

bool WeakScheduler::schedule(const util::SimpleIdentity tag, std::function<void()>&& fn) const {
    const auto locked = anchor.lock();
    if (!locked || locked->expiring) {
        return false;
    }
    locked->scheduler.schedule(tag, std::move(fn));
    return true;
}

Scheduler::Scheduler()
    : anchor(std::make_shared<WeakScheduler::Anchor>(*this)) {}

Scheduler::~Scheduler() {
    assert(!anchor && "Call expireWeakSchedulers() first in the destructor of every concrete scheduler");
    expireWeakSchedulers();
}

void Scheduler::expireWeakSchedulers() {
    if (!anchor) {
        return;
    }
    anchor->expiring = true;
    auto released = anchor->released.get_future();
    anchor.reset();
    released.wait();
}

std::function<void()> Scheduler::bindOnce(std::function<void()> fn) const {
    assert(fn);
    return [scheduler = makeWeak(), scheduled = std::move(fn)]() mutable {
        if (!scheduled) return; // Repeated call.
        scheduler.schedule(std::move(scheduled));
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

    std::scoped_lock lock(mtx);
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

    std::scoped_lock lock(mtx);

    lastUsedIndex = (lastUsedIndex + 1) % kSchedulersCount;

    if (auto scheduler = weaks[lastUsedIndex].lock()) {
        return scheduler;
    } else {
        auto result = std::make_shared<SequencedScheduler>();
        weaks[lastUsedIndex] = result;
        return result;
    }
}

} // namespace mln
