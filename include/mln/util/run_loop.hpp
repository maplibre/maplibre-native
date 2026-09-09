#pragma once

#include <mln/actor/scheduler.hpp>
#include <mln/actor/mailbox.hpp>
#include <mln/util/chrono.hpp>
#include <mln/util/noncopyable.hpp>
#include <mln/util/scoped.hpp>
#include <mln/util/util.hpp>
#include <mln/util/work_task.hpp>
#include <mln/util/work_request.hpp>

#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace mln {
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

    /// Like runOnce(), but stops dequeuing tasks once `budget` has elapsed.
    /// At least one queued task runs. Tasks left behind stay queued and the
    /// loop is woken again, so the next runOnce() or run() picks them up.
    void runOnce(Duration budget) {
        processDeadline = Clock::now() + budget;
        const Scoped clearDeadline([this] { processDeadline.reset(); });
        runOnce();
    }
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
        bool ranTask = false;
        while (true) {
            if (highPriorityQueue.empty() && defaultQueue.empty()) {
                break;
            }
            if (ranTask && processDeadline && Clock::now() >= *processDeadline) {
                // Re-arm the wake so the remaining tasks run on the next iteration.
                wake();
                break;
            }
            if (!highPriorityQueue.empty()) {
                task = std::move(highPriorityQueue.front());
                highPriorityQueue.pop();
            } else {
                task = std::move(defaultQueue.front());
                defaultQueue.pop();
            }
            lock.unlock();
            (*task)();
            task.reset();
            lock.lock();
            ranTask = true;
        }
    }

    std::function<void()> platformCallback;
    std::optional<TimePoint> processDeadline;

    Queue defaultQueue;
    Queue highPriorityQueue;
    std::mutex mutex;

    std::unique_ptr<Impl> impl;
    ::mapbox::base::WeakPtrFactory<Scheduler> weakFactory{this};
    // Do not add members here, see `WeakPtrFactory`
};

} // namespace util
} // namespace mln

#include <mln/util/work_task_impl.hpp>
