#include <mbgl/util/thread_pool.hpp>

#include <mbgl/platform/settings.hpp>
#include <mbgl/platform/thread.hpp>
#include <mbgl/util/monotonic_timer.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/string.hpp>

namespace mbgl {

ThreadedSchedulerBase::~ThreadedSchedulerBase() = default;

void ThreadedSchedulerBase::terminate() {
    {
        std::lock_guard<std::mutex> lock(workerMutex);
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

        owningThreadPool.set(this);

        bool didWork = true;
        while (true) {
            std::unique_lock<std::mutex> conditionLock(workerMutex);
            if (!terminated && !didWork) {
                cvAvailable.wait(conditionLock);
            }

            if (terminated) {
                platform::detachThread();
                break;
            }

            // Let other threads run
            conditionLock.unlock();

            didWork = false;
            std::vector<std::shared_ptr<Queue>> pending;
            {
                // 1. Gather buckets for us to visit this iteration
                std::lock_guard<std::mutex> lock(taggedQueueLock);
                for (const auto& [tag, queue] : taggedQueue) {
                    pending.push_back(queue);
                }
            }

            // 2. Visit a task from each
            for (auto& q : pending) {
                std::function<void()> tasklet;
                {
                    std::lock_guard<std::mutex> lock(q->lock);
                    if (q->queue.size()) {
                        tasklet = std::move(q->queue.front());
                        q->queue.pop();
                    }
                    if (!tasklet) continue;
                }

                q->runningCount++;

                try {
                    // Indicate some processing was done this iteration. We'll try and do more work
                    // on the following loop until we run out of work, at which point we wait.
                    didWork = true;

                    tasklet();
                    tasklet = {}; // destroy the function and release its captures before unblocking `waitForEmpty`

                    if (!--q->runningCount) {
                        std::lock_guard<std::mutex> lock(q->lock);
                        if (q->queue.empty()) {
                            q->cv.notify_all();
                        }
                    }
                } catch (...) {
                    std::lock_guard<std::mutex> lock(q->lock);
                    if (handler) {
                        handler(std::current_exception());
                    }

                    tasklet = {};

                    if (!--q->runningCount && q->queue.empty()) {
                        q->cv.notify_all();
                    }

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
    schedule(static_cast<const void*>(this), std::move(fn));
}

void ThreadedSchedulerBase::schedule(const void* tag, std::function<void()>&& fn) {
    assert(fn);
    if (!fn) return;

    std::shared_ptr<Queue> q;
    {
        std::lock_guard<std::mutex> lock(taggedQueueLock);
        auto it = taggedQueue.find(tag);
        if (it == taggedQueue.end()) {
            q = std::make_shared<Queue>();
            taggedQueue.insert({tag, q});
        } else {
            q = it->second;
        }
    }

    {
        std::lock_guard<std::mutex> lock(q->lock);
        q->queue.push(std::move(fn));
    }

    cvAvailable.notify_one();
}

void ThreadedSchedulerBase::waitForEmpty(const void* tag) {
    // Must not be called from a thread in our pool, or we would deadlock
    assert(!thisThreadIsOwned());
    if (!thisThreadIsOwned()) {
        if (!tag) {
            tag = static_cast<const void*>(this);
        }

        std::shared_ptr<Queue> q;
        {
            std::lock_guard<std::mutex> lock(taggedQueueLock);
            auto it = taggedQueue.find(tag);
            if (it == taggedQueue.end()) {
                return;
            }
            q = it->second;
        }

        std::unique_lock<std::mutex> queueLock(q->lock);
        while (q->queue.size() + q->runningCount) {
            q->cv.wait(queueLock);
        }

        // After waiting for the queue to empty, go ahead and erase it from the map.
        {
            std::lock_guard<std::mutex> lock(taggedQueueLock);
            taggedQueue.erase(tag);
        }
    }
}

} // namespace mbgl
