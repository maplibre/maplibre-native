#include "scheduler.hpp"

#include <mbgl/util/monotonic_timer.hpp>
#include <mbgl/util/util.hpp>

#include <cassert>

namespace QMapLibre {

Scheduler::Scheduler() = default;

Scheduler::~Scheduler() {
    MBGL_VERIFY_THREAD(tid);
}

void Scheduler::schedule(const mbgl::util::SimpleIdentity /*id*/, std::function<void()>&& function) {
    const std::lock_guard<std::mutex> lock(m_taskQueueMutex);
    m_taskQueue.push(std::move(function));

    // Need to force the main thread to wake
    // up this thread and process the events.
    emit needsProcessing();
}

void Scheduler::schedule(std::function<void()>&& function) {
    schedule(mbgl::util::SimpleIdentity::Empty, std::move(function));
}

void Scheduler::processEvents() {
    std::queue<std::function<void()>> taskQueue;
    {
        const std::unique_lock<std::mutex> lock(m_taskQueueMutex);
        std::swap(taskQueue, m_taskQueue);
        pendingItems += taskQueue.size();
    }

    while (!taskQueue.empty()) {
        auto& function = taskQueue.front();
        if (function) {
            function();
        }
        taskQueue.pop();
        pendingItems--;
    }

    cvEmpty.notify_all();
}

void Scheduler::waitForEmpty([[maybe_unused]] const mbgl::util::SimpleIdentity tag) {
    MBGL_VERIFY_THREAD(tid);

    std::unique_lock<std::mutex> lock(m_taskQueueMutex);
    const auto isDone = [&] {
        return m_taskQueue.empty() && pendingItems == 0;
    };

    while (!isDone()) {
        cvEmpty.wait(lock);
    }

    assert(m_taskQueue.size() + pendingItems == 0);
}

} // namespace QMapLibre
