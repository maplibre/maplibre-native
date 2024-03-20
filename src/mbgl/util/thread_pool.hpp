#pragma once

#include <mbgl/actor/mailbox.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/thread_local.hpp>

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace mbgl {

class ThreadedSchedulerBase : public Scheduler {
public:
    void schedule(std::function<void()>&&) override;

protected:
    ThreadedSchedulerBase() = default;
    ~ThreadedSchedulerBase() override;

    void terminate();
    std::thread makeSchedulerThread(size_t index);

    /// Wait until there's nothing pending or in process
    /// Must not be called from a task provided to this scheduler.
    /// @param timeout Time to wait, or zero to wait forever.
    std::size_t waitForEmpty(Milliseconds timeout) override;

    /// Returns true if called from a thread managed by the scheduler
    bool thisThreadIsOwned() const { return owningThreadPool.get() == this; }

    std::queue<std::function<void()>> queue;
    // protects `queue`
    std::mutex mutex;
    // Used to block addition of new items while waiting
    std::mutex addMutex;
    // Signal when an item is added to the queue
    std::condition_variable cvAvailable;
    // Signal when the queue becomes empty
    std::condition_variable cvEmpty;
    // Count of functions removed from the queue but still executing
    std::atomic<std::size_t> pendingItems{0};
    // Points to the owning pool in owned threads
    util::ThreadLocal<ThreadedSchedulerBase> owningThreadPool;
    bool terminated{false};
};

/**
 * @brief ThreadScheduler implements Scheduler interface using a lightweight event loop
 *
 * @tparam N number of threads
 *
 * Note: If N == 1 all scheduled tasks are guaranteed to execute consequently;
 * otherwise, some of the scheduled tasks might be executed in parallel.
 */
class ThreadedScheduler : public ThreadedSchedulerBase {
public:
    ThreadedScheduler(std::size_t n)
        : threads(n) {
        for (std::size_t i = 0u; i < threads.size(); ++i) {
            threads[i] = makeSchedulerThread(i);
        }
    }

    ~ThreadedScheduler() override {
        assert(!thisThreadIsOwned());
        terminate();
        for (auto& thread : threads) {
            assert(std::this_thread::get_id() != thread.get_id());
            thread.join();
        }
    }

    void runOnRenderThread(std::function<void()>&& fn) override {
        std::lock_guard<std::mutex> lock(renderMutex);
        renderThreadQueue.push(std::move(fn));
    }

    void runRenderJobs() override {
        std::lock_guard<std::mutex> lock(renderMutex);
        while (renderThreadQueue.size()) {
            auto fn = std::move(renderThreadQueue.front());
            renderThreadQueue.pop();
            if (fn) {
                fn();
            }
        }
    }

    mapbox::base::WeakPtr<Scheduler> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

private:
    std::vector<std::thread> threads;
    mapbox::base::WeakPtrFactory<Scheduler> weakFactory{this};

    std::queue<std::function<void()>> renderThreadQueue;
    std::mutex renderMutex;
};

class SequencedScheduler : public ThreadedScheduler {
public:
    SequencedScheduler()
        : ThreadedScheduler(1) {}
};

class ParallelScheduler : public ThreadedScheduler {
public:
    ParallelScheduler(std::size_t extra)
        : ThreadedScheduler(1 + extra) {}
};

class ThreadPool : public ParallelScheduler {
public:
    ThreadPool()
        : ParallelScheduler(3) {}
};

} // namespace mbgl
