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
            {
                std::unique_lock<std::mutex> lock(mutex);
                cvAvailable.wait(lock, [this] { return !queue.empty() || terminated; });

                if (terminated) {
                    platform::detachThread();
                    return;
                }

                auto function = std::move(queue.front());
                queue.pop();

                execPending++;
                try {
                    lock.unlock();

                    if (function) {
                        function();
                    }

                    // destroy the function and release its captures before unblocking `waitForEmpty`
                    function = {};
                    execPending--;
                } catch (...) {
                    execPending--;
                    throw;
                }
            }

            if (queue.empty()) {
                cvEmpty.notify_all();
            }
        }
    });
}

void ThreadedSchedulerBase::schedule(std::function<void()>&& fn) {
    assert(fn);
    if (fn) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(std::move(fn));
        }
        cvAvailable.notify_one();
    }
}

std::size_t ThreadedSchedulerBase::waitForEmpty(std::chrono::milliseconds timeout) {
    if (mainThreadID != std::this_thread::get_id()) {
        assert(false);
        return false;
    }

    const auto startTime = mbgl::util::MonotonicTimer::now();
    std::unique_lock<std::mutex> lock(mutex);
    while (!queue.empty() || execPending.load() > 0) {
        const auto elapsed = mbgl::util::MonotonicTimer::now() - startTime;
        auto isDone = [&] {
            return queue.empty() && execPending.load() == 0;
        };
        if (timeout <= elapsed || !cvEmpty.wait_for(lock, timeout - elapsed, std::move(isDone))) {
            assert(queue.empty());
            break;
        }
    }

    return queue.size();
}

} // namespace mbgl
