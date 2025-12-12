#include <mbgl/util/thread_pool.hpp>

#include <mbgl/platform/settings.hpp>
#include <mbgl/platform/thread.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/monotonic_timer.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/string.hpp>

namespace mbgl {

ThreadedSchedulerBase::~ThreadedSchedulerBase() = default;

void ThreadedSchedulerBase::terminate() {
    {
        std::scoped_lock lock(workerMutex);
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

        while (true) {
            std::unique_lock<std::mutex> conditionLock(workerMutex);
            if (!terminated && taskCount == 0) {
                cvAvailable.wait(conditionLock);
            }

            if (terminated) {
                platform::detachThread();
                break;
            }

            // Let other threads run
            conditionLock.unlock();

            std::vector<std::shared_ptr<Queue>> pending;
            {
                // 1. Gather buckets for us to visit this iteration
                std::scoped_lock lock(taggedQueueLock);
                for (const auto& [tag, queue] : taggedQueue) {
                    pending.push_back(queue);
                }
            }

            // 2. Visit a task from each
            for (auto& q : pending) {
                std::function<void()> tasklet;
                {
                    std::scoped_lock lock(q->lock);
                    if (q->queue.size()) {
                        q->runningCount++;
                        tasklet = std::move(q->queue.front());
                        q->queue.pop();
                    }
                    if (!tasklet) continue;
                }

                assert(taskCount > 0);
                taskCount--;

                try {
                    tasklet();
                    tasklet = {}; // destroy the function and release its captures before unblocking `waitForEmpty`

                    if (!--q->runningCount) {
                        std::scoped_lock lock(q->lock);
                        if (q->queue.empty()) {
                            q->cv.notify_all();
                        }
                    }
                } catch (...) {
                    std::scoped_lock lock(q->lock);
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
    schedule(uniqueID, std::move(fn));
}

void ThreadedSchedulerBase::schedule(const util::SimpleIdentity tag, std::function<void()>&& fn) {
    MLN_TRACE_FUNC();
    assert(fn);
    if (!fn) return;

    std::shared_ptr<Queue> q;
    {
        MLN_TRACE_ZONE(queue);
        std::scoped_lock lock(taggedQueueLock);

        // find or insert
        auto result = taggedQueue.insert(std::make_pair(tag, std::shared_ptr<Queue>{}));
        if (result.second) {
            // new entry inserted
            result.first->second = std::make_shared<Queue>();
        }
        q = result.first->second;

        MLN_ZONE_VALUE(taggedQueue.size());
    }

    {
        MLN_TRACE_ZONE(push);
        std::scoped_lock lock(q->lock);
        q->queue.push(std::move(fn));
        taskCount++;
    }

    // Take the worker lock before notifying to prevent threads from waiting while we try to wake them
    std::scoped_lock workerLock(workerMutex);
    cvAvailable.notify_one();
}

void ThreadedSchedulerBase::waitForEmpty(const util::SimpleIdentity tag) {
    // Must not be called from a thread in our pool, or we would deadlock
    assert(!thisThreadIsOwned());
    if (!thisThreadIsOwned()) {
        const auto tagToFind = tag.isEmpty() ? uniqueID : tag;

        std::shared_ptr<Queue> q;
        {
            std::scoped_lock lock(taggedQueueLock);
            auto it = taggedQueue.find(tagToFind);
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
            std::scoped_lock lock(taggedQueueLock);
            taggedQueue.erase(tagToFind);
        }
    }
}

} // namespace mbgl
