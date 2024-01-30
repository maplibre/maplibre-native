#include <mbgl/util/thread_pool.hpp>

#include <mbgl/platform/settings.hpp>
#include <mbgl/platform/thread.hpp>
#include <mbgl/util/monotonic_timer.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/string.hpp>

namespace mbgl {

ThreadedSchedulerBase::~ThreadedSchedulerBase() = default;

void ThreadedSchedulerBase::terminate() {
    // Run any leftover render jobs
    runRenderJobs();

    {
        std::lock_guard<std::mutex> lock(mutex);
        terminated = true;
    }

    // Wake up all threads so that they shut down
    cvAvailable.notify_all();
}

std::thread ThreadedSchedulerBase::makeSchedulerThread(size_t index) {
    return std::thread([this, index] {
        auto& settings = platform::Settings::getInstance();
        auto value = settings.get(platform::EXPERIMENTAL_THREAD_PRIORITY_WORKER);
        if (auto* priority = value.getDouble()) {
            platform::setCurrentThreadPriority(*priority);
        }

        platform::setCurrentThreadName("Worker " + util::toString(index + 1));
        platform::attachThread();

        while (true) {
            std::unique_lock<std::mutex> lock(mutex);
            if (queue.empty() && !pendingItems) {
                cvEmpty.notify_all();
            }

            cvAvailable.wait(lock, [this] { return !queue.empty() || terminated; });

            if (terminated) {
                platform::detachThread();
                return;
            }

            auto function = std::move(queue.front());
            queue.pop();

            if (function) {
                pendingItems++;
            }

            lock.unlock();

            if (function) {
                const auto cleanup = [&] {
                    // destroy the function and release its captures before unblocking `waitForEmpty`
                    function = {};
                    pendingItems--;
                    if (queue.empty() && !pendingItems) {
                        cvEmpty.notify_all();
                    }
                };
                try {
                    function();
                    cleanup();
                } catch (...) {
                    lock.lock();
                    if (handler) {
                        handler(std::current_exception());
                    }
                    cleanup();
                    if (handler) {
                        continue;
                    }
                    throw;
                }
            }
        }
    });
}

void ThreadedSchedulerBase::schedule(std::function<void()>&& fn) {
    assert(fn);
    if (fn) {
        {
            // We need to block if adding adding a new task from a thread not controlled by this
            // pool.  Tasks are added by other tasks, so we must not block a thread we do control
            // or `waitForEmpty` will deadlock.
            std::unique_lock<std::mutex> addLock(addMutex, std::defer_lock);
            if (!isThreadOwned(std::this_thread::get_id())) {
                addLock.lock();
            }
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(std::move(fn));
        }
        cvAvailable.notify_one();
    }
}

std::size_t ThreadedSchedulerBase::waitForEmpty(Milliseconds timeout) {
    // Must not be called from a thread in our pool, or we would deadlock
    if (!isThreadOwned(std::this_thread::get_id())) {
        const auto startTime = util::MonotonicTimer::now();
        const auto isDone = [&] {
            return queue.empty() && pendingItems == 0;
        };
        // Block any other threads from adding new items
        std::scoped_lock<std::mutex> addLock(addMutex);
        std::unique_lock<std::mutex> lock(mutex);
        while (!isDone()) {
            if (timeout > Milliseconds::zero()) {
                const auto elapsed = util::MonotonicTimer::now() - startTime;
                if (timeout <= elapsed || !cvEmpty.wait_for(lock, timeout - elapsed, isDone)) {
                    break;
                }
            } else {
                cvEmpty.wait(lock, isDone);
            }
        }
        return queue.size() + pendingItems;
    }
    assert(false);
    return 0;
}

} // namespace mbgl
