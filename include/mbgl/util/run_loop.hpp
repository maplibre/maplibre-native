#pragma once

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/actor/mailbox.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/util.hpp>
#include <mbgl/util/work_task.hpp>
#include <mbgl/util/work_request.hpp>

#include <functional>
#include <mutex>
#include <queue>
#include <utility>

namespace mbgl {
namespace util {

using LOOP_HANDLE = void*;

// NOTE: Any derived class must invalidate `weakFactory` in the destructor
class RunLoop final : public Scheduler, private util::noncopyable {
public:
    enum class Type : uint8_t {
        Default,
        New,
    };

    enum class Priority : bool {
        Default = false,
        High = true,
    };

    enum class Event : uint8_t {
        None = 0,
        Read = 1,
        Write = 2,
        ReadWrite = Read | Write,
    };

    RunLoop(Type type = Type::Default);
    ~RunLoop() override;

    static RunLoop* Get();
    static LOOP_HANDLE getLoopHandle();

    void run();
    void runOnce();
    void stop();

    void updateTime();

    /// Platform integration callback for platforms that do not have full
    /// run loop integration or don't want to block at the Mapbox GL Native
    /// loop. It will be called from any thread and is up to the platform
    /// to, after receiving the callback, call RunLoop::runOnce() from the
    /// same thread as the Map object lives.
    void setPlatformCallback(std::function<void()> callback) { platformCallback = std::move(callback); }

    // So far only needed by the libcurl backend.
    void addWatch(int fd, Event, std::function<void(int, Event)>&& callback);
    void removeWatch(int fd);

    // Invoke fn(args...) on this RunLoop.
    template <class Fn, class... Args>
    void invoke(Priority priority, Fn&& fn, Args&&... args) {
        push(priority, WorkTask::make(std::forward<Fn>(fn), std::forward<Args>(args)...));
    }

    // Invoke fn(args...) on this RunLoop.
    template <class Fn, class... Args>
    void invoke(Fn&& fn, Args&&... args) {
        invoke(Priority::Default, std::forward<Fn>(fn), std::forward<Args>(args)...);
    }

    // Post the cancellable work fn(args...) to this RunLoop.
    template <class Fn, class... Args>
    std::unique_ptr<AsyncRequest> invokeCancellable(Fn&& fn, Args&&... args) {
        std::shared_ptr<WorkTask> task = WorkTask::make(std::forward<Fn>(fn), std::forward<Args>(args)...);
        push(Priority::Default, task);
        return std::make_unique<WorkRequest>(task);
    }

    void schedule(std::function<void()>&& fn) override { invoke(std::move(fn)); }
    void schedule(const util::SimpleIdentity, std::function<void()>&& fn) override { schedule(std::move(fn)); }
    ::mapbox::base::WeakPtr<Scheduler> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

    void waitForEmpty(const util::SimpleIdentity = util::SimpleIdentity::Empty) override;

    class Impl;

    // Allows derived classes to invalidate weak pointers in
    // their destructor before their own members are torn down.
    void invalidateWeakPtrsEarly() { weakFactory.invalidateWeakPtrs(); }

private:
    MBGL_STORE_THREAD(tid)

    using Queue = std::queue<std::shared_ptr<WorkTask>>;

    // Wakes up the RunLoop so that it starts processing items in the queue.
    void wake();

    // Adds a WorkTask to the queue, and wakes it up.
    void push(Priority priority, std::shared_ptr<WorkTask> task) {
        std::scoped_lock lock(mutex);
        if (priority == Priority::High) {
            highPriorityQueue.emplace(std::move(task));
        } else {
            defaultQueue.emplace(std::move(task));
        }
        wake();

        if (platformCallback) {
            platformCallback();
        }
    }

    void process() {
        std::shared_ptr<WorkTask> task;
        std::unique_lock<std::mutex> lock(mutex);
        while (true) {
            if (!highPriorityQueue.empty()) {
                task = std::move(highPriorityQueue.front());
                highPriorityQueue.pop();
            } else if (!defaultQueue.empty()) {
                task = std::move(defaultQueue.front());
                defaultQueue.pop();
            } else {
                break;
            }
            lock.unlock();
            (*task)();
            task.reset();
            lock.lock();
        }
    }

    std::function<void()> platformCallback;

    Queue defaultQueue;
    Queue highPriorityQueue;
    std::mutex mutex;

    std::unique_ptr<Impl> impl;
    ::mapbox::base::WeakPtrFactory<Scheduler> weakFactory{this};
    // Do not add members here, see `WeakPtrFactory`
};

} // namespace util
} // namespace mbgl

#include <mbgl/util/work_task_impl.hpp>
