#pragma once

#include <mbgl/util/identity.hpp>

#include <mapbox/std/weak.hpp>

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace mbgl {

/**
    A `Scheduler` is responsible for coordinating the processing of messages by
    one or more actors via their mailboxes. It's an abstract interface. Currently,
    the following concrete implementations exist:

    * `ThreadPool` can coordinate an unlimited number of actors over any number of
      threads via a pool, preserving the following behaviors:

      - Messages from each individual mailbox are processed in order
      - Only a single message from a mailbox is processed at a time; there is no
        concurrency within a mailbox

      Subject to these constraints, processing can happen on whatever thread in the
      pool is available.

    * `Scheduler::GetCurrent()` is typically used to create a mailbox and `ActorRef`
      for an object that lives on the main thread and is not itself wrapped an
      `Actor`. The underlying implementation of this Scheduler should usually be
      a `RunLoop`
        auto mailbox = std::make_shared<Mailbox>(*Scheduler::Get());
        Actor<Worker> worker(threadPool, ActorRef<Foo>(*this, mailbox));
*/
class Scheduler {
public:
    virtual ~Scheduler() {
        std::unique_lock<std::mutex> counterLock(deferredSignalLock);
        while (deferredDeletionsPending > 0) {
            deferredSignal.wait(counterLock);
        }
    }

    /// Enqueues a function for execution.
    virtual void schedule(std::function<void()>&&) = 0;
    virtual void schedule(const util::SimpleIdentity, std::function<void()>&&) = 0;

    /// Makes a weak pointer to this Scheduler.
    virtual mapbox::base::WeakPtr<Scheduler> makeWeakPtr() = 0;
    /// Enqueues a function for execution on the render thread owned by the given tag.
    virtual void runOnRenderThread(const util::SimpleIdentity, std::function<void()>&&) {}
    /// Run render thread jobs for the given tag
    /// @param tag Tag of owner
    /// @param closeQueue Runs all render jobs and then removes the internal queue.
    virtual void runRenderJobs([[maybe_unused]] const util::SimpleIdentity tag,
                               [[maybe_unused]] bool closeQueue = false) {}
    /// Returns a closure wrapping the given one.
    ///
    /// When the returned closure is invoked for the first time, it schedules
    /// the given closure to this scheduler, the consequent calls of the
    /// returned closure are ignored.
    ///
    /// If this scheduler is already deleted by the time the returnded closure
    /// is first invoked, the call is ignored.
    std::function<void()> bindOnce(std::function<void()>);

    /// Enqueues the given |task| for execution into this scheduler's task queue
    /// and then enqueues the |reply| with the captured task result to the
    /// current task queue.
    ///
    /// The |TaskFn| return type must be compatible with the |ReplyFn| argument
    /// type. Note: the task result is copied and passed by value.
    template <typename TaskFn, typename ReplyFn>
    void scheduleAndReplyValue(const util::SimpleIdentity tag, TaskFn&& task, ReplyFn&& reply) {
        assert(GetCurrent());
        static_assert(std::is_invocable_v<TaskFn>);
        scheduleAndReplyValue(
            tag, std::forward<TaskFn>(task), std::forward<ReplyFn>(reply), GetCurrent()->makeWeakPtr());
    }

    /// Wait until there's nothing pending or in process
    /// Must not be called from a task provided to this scheduler.
    virtual void waitForEmpty(const util::SimpleIdentity = util::SimpleIdentity::Empty) = 0;

    /// Set/Get the current Scheduler for this thread
    static Scheduler* GetCurrent();
    static void SetCurrent(Scheduler*);

    /// Get the scheduler for asynchronous tasks. This method
    /// will lazily initialize a shared worker pool when ran
    /// from the first time.
    /// The scheduled tasks might run in parallel on different
    /// threads.
    /// TODO : Rename to GetPool()
    [[nodiscard]] static std::shared_ptr<Scheduler> GetBackground();

    /// Get the *sequenced* scheduler for asynchronous tasks.
    /// Unlike the method above, the returned scheduler
    /// (once stored) represents a single thread, thus each
    /// newly scheduled task is guarantied to run after the
    /// previously scheduled one.
    ///
    /// Sequenced scheduler can be used for running tasks
    /// on the same thread-unsafe object.
    [[nodiscard]] static std::shared_ptr<Scheduler> GetSequenced();

    /// Set a function to be called when an exception occurs on a thread controlled by the scheduler
    void setExceptionHandler(std::function<void(const std::exception_ptr)> handler_) { handler = std::move(handler_); }

    /// Capture an object and release it on a thread in the pool.
    /// @tparam T The arbitrary type to be captured
    /// @tparam TWrap The type of the owning wrapper (`shared_ptr`, `Immutable`, `Mutable`, ...)
    ///
    /// To be used with a `unique_ptr` it must first be converted to a `shared_ptr`, because we have
    /// be able to make a copy in the process of capturing it in a lambda.
    template <typename T>
    void deferredRelease(const util::SimpleIdentity tag, T&& item) {
        // The `std::function` must be created in a separate statement from the `schedule` call.
        // Creating a `std::function` from a lambda involves a copy, which is why we must use
        // `shared_ptr` rather than `unique_ptr` for the capture.  As a result, a temporary holds
        // a reference until the construction is complete and the lambda is destroyed.
        // If this temporary outlives the `schedule` call, and the function is executed immediately
        // by a waiting thread and is already complete, that temporary reference ends up being the
        // last one and the destruction actually occurs here on this thread.
        std::function<void()> func{[owner_{CaptureWrapper<T>{std::forward<T>(item)}}, this]() mutable {
            {
                // move to local and destroy, since not everything allows assignment of `{}`
                T temporary{std::move(owner_.item)};
            }
            std::lock_guard<std::mutex> counterLock(deferredSignalLock);
            deferredDeletionsPending--;
            deferredSignal.notify_all();
        }};

        {
            std::lock_guard<std::mutex> counterLock(deferredSignalLock);
            deferredDeletionsPending++;
        }

        schedule(tag, std::move(func));
    }

    /// Transfer ownership to a capture that's released on the render thread
    template <typename T>
    void releaseOnRenderThread(const util::SimpleIdentity tag, T&& owner) {
        std::function<void()> func{[owner_{CaptureWrapper<T>{std::forward<T>(owner)}}]() mutable {
        }};
        runOnRenderThread(tag, std::move(func));
    }

protected:
    std::function<void(const std::exception_ptr)> handler;

private:
    size_t deferredDeletionsPending{0};
    std::mutex deferredSignalLock;
    std::condition_variable deferredSignal;

    /// This exists solely to prevent a problem where temporary lambda captures
    /// are retained for the duration of the scope instead of being destroyed immediately.
    template <typename T>
    struct CaptureWrapper {
        CaptureWrapper(T&& item_)
                : item(std::move(item_)) {}
        CaptureWrapper(const CaptureWrapper& other)
                : item(other.item) {}
        CaptureWrapper(CaptureWrapper&& other)
                : item(std::move(other.item)) {}
        T item;
    };

    template <typename TaskFn, typename ReplyFn>
    void scheduleAndReplyValue(const util::SimpleIdentity tag,
                               TaskFn&& task,
                               ReplyFn&& reply,
                               mapbox::base::WeakPtr<Scheduler> replyScheduler) {
        schedule(tag, [replyScheduler = std::move(replyScheduler), tag, task, reply] {
            auto lock = replyScheduler.lock();
            if (!replyScheduler) return;
            replyScheduler->schedule(tag, [reply, result = task()] { reply(result); });
        });
    }
};

/// @brief A TaggedScheduler pairs a scheduler with an identifier. Tasklets submitted via a TaggedScheduler
/// are bucketed with the tag to enable queries on tasks related to that tag. This allows multiple map
/// instances to all use the same scheduler and await processing of all their tasks prior to map deletion.
class TaggedScheduler {
public:
    TaggedScheduler() = delete;
    TaggedScheduler(std::shared_ptr<Scheduler> scheduler_, const util::SimpleIdentity tag_)
        : tag(tag_),
          scheduler(std::move(scheduler_)) {}
    TaggedScheduler(const TaggedScheduler&) = default;

    /// @brief Get the wrapped scheduler
    const std::shared_ptr<Scheduler>& get() const noexcept { return scheduler; }

    void schedule(std::function<void()>&& fn) { scheduler->schedule(tag, std::move(fn)); }
    void runOnRenderThread(std::function<void()>&& fn) { scheduler->runOnRenderThread(tag, std::move(fn)); }
    void runRenderJobs(bool closeQueue = false) { scheduler->runRenderJobs(tag, closeQueue); }
    void waitForEmpty() const noexcept { scheduler->waitForEmpty(tag); }

    /// type. Note: the task result is copied and passed by value.
    template <typename TaskFn, typename ReplyFn>
    void scheduleAndReplyValue(TaskFn&& task, ReplyFn&& reply) {
        scheduler->scheduleAndReplyValue(tag, task, reply);
    }

    /// Destroy a tile without blocking
    template <typename T>
    void deferredRelease(T&& item) {
        scheduler->deferredRelease(tag, std::forward<T>(item));
    }

    template <typename T>
    void releaseOnRenderThread(T&& owner) {
        scheduler->releaseOnRenderThread<T>(tag, std::forward<T>(owner));
    }

    const mbgl::util::SimpleIdentity tag;

private:
    std::shared_ptr<Scheduler> scheduler;
};

} // namespace mbgl
