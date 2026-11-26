#include <mln/actor/scheduler.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/thread_pool.hpp>

#include <mln/test/util.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <vector>

using namespace mln;
using namespace std::chrono_literals;

namespace {

// Runs scheduled functions synchronously, so that a test can control when `schedule()` returns.
class SyncScheduler final : public Scheduler {
public:
    explicit SyncScheduler(std::function<void()> onSchedule_ = {})
        : onSchedule(std::move(onSchedule_)) {}

    ~SyncScheduler() override {
        expireWeakSchedulers();
        alive = false;
    }

    void schedule(std::function<void()>&& fn) override {
        EXPECT_TRUE(alive);
        if (onSchedule) onSchedule();
        fn();
    }

    void schedule(const util::SimpleIdentity, std::function<void()>&& fn) override { schedule(std::move(fn)); }

    void waitForEmpty(const util::SimpleIdentity) override {}

    std::atomic<bool> alive{true};

private:
    std::function<void()> onSchedule;
};

} // namespace

TEST(WeakScheduler, Empty) {
    const WeakScheduler weak;
    EXPECT_TRUE(weak.expired());

    bool ran = false;
    std::function<void()> fn = [&] {
        ran = true;
    };
    EXPECT_FALSE(weak.schedule(std::move(fn)));
    EXPECT_TRUE(fn);
    EXPECT_FALSE(ran);
}

TEST(WeakScheduler, SchedulesUntilSchedulerIsDestroyed) {
    auto scheduler = std::make_unique<SyncScheduler>();
    const auto weak = scheduler->makeWeak();
    EXPECT_FALSE(weak.expired());

    int runs = 0;
    EXPECT_TRUE(weak.schedule([&] { ++runs; }));
    EXPECT_TRUE(weak.schedule(util::SimpleIdentity{}, [&] { ++runs; }));
    EXPECT_EQ(2, runs);

    scheduler.reset();
    EXPECT_TRUE(weak.expired());

    std::function<void()> fn = [&] {
        ++runs;
    };
    EXPECT_FALSE(weak.schedule(std::move(fn)));
    EXPECT_FALSE(weak.schedule(util::SimpleIdentity{}, std::move(fn)));
    EXPECT_TRUE(fn);
    EXPECT_EQ(2, runs);
}

TEST(WeakScheduler, Equality) {
    SyncScheduler first;
    SyncScheduler second;

    EXPECT_EQ(first.makeWeak(), first.makeWeak());
    EXPECT_NE(first.makeWeak(), second.makeWeak());
    EXPECT_NE(first.makeWeak(), WeakScheduler());
    EXPECT_EQ(WeakScheduler(), WeakScheduler());
}

TEST(WeakScheduler, DestructionWaitsForScheduleInProgress) {
    std::promise<void> entered;
    std::promise<void> proceed;
    auto proceedFuture = proceed.get_future();
    std::atomic<bool> ran{false};

    auto scheduler = std::make_unique<SyncScheduler>([&] {
        entered.set_value();
        proceedFuture.wait();
    });
    const auto weak = scheduler->makeWeak();

    std::thread poster([&] { EXPECT_TRUE(weak.schedule([&] { ran = true; })); });
    entered.get_future().wait();

    std::atomic<bool> destroyed{false};
    std::thread destroyer([&] {
        scheduler.reset();
        EXPECT_TRUE(ran);
        destroyed = true;
    });

    std::this_thread::sleep_for(50ms);
    EXPECT_FALSE(destroyed);

    proceed.set_value();
    poster.join();
    destroyer.join();
    EXPECT_TRUE(destroyed);
    EXPECT_TRUE(weak.expired());
}

TEST(WeakScheduler, ConcurrentPostsWhileSchedulerIsDestroyed) {
    // SyncScheduler fails the test if it is scheduled on once its destruction started.
    auto scheduler = std::make_unique<SyncScheduler>();
    const auto weak = scheduler->makeWeak();

    std::atomic<bool> stop{false};
    std::atomic<std::size_t> rejected{0};
    std::vector<std::thread> posters;
    for (int i = 0; i < 4; ++i) {
        posters.emplace_back([&] {
            while (!stop) {
                if (!weak.schedule([] {})) {
                    ++rejected;
                }
            }
        });
    }

    std::this_thread::sleep_for(10ms);
    scheduler.reset();
    while (rejected < 1000) {
        std::this_thread::yield();
    }
    stop = true;
    for (auto& poster : posters) {
        poster.join();
    }
}

TEST(WeakScheduler, BindOnceAfterSchedulerIsDestroyed) {
    auto scheduler = std::make_unique<SyncScheduler>();
    bool ran = false;
    auto bound = scheduler->bindOnce([&] { ran = true; });

    scheduler.reset();
    bound();
    EXPECT_FALSE(ran);
}

TEST(WeakScheduler, ReplySchedulerDestroyedWhileTaskRuns) {
    // Destroying the scheduler a reply is for does not wait for the task, and drops the reply.
    const util::SimpleIdentity tag;
    auto pool = Scheduler::GetBackground();
    auto loop = std::make_unique<util::RunLoop>();

    std::promise<void> entered;
    std::promise<void> proceed;
    std::shared_future<void> proceedFuture = proceed.get_future().share();
    std::atomic<bool> replied{false};
    pool->scheduleAndReplyValue(
        tag,
        [&entered, proceedFuture] {
            entered.set_value();
            proceedFuture.wait();
            return 1;
        },
        [&](int) { replied = true; });
    entered.get_future().wait();

    std::atomic<bool> loopDestroyed{false};
    std::thread timeout([&] {
        // Lets the task finish if destroying the loop waits for it, so that the test fails instead of hanging.
        for (int i = 0; i < 500 && !loopDestroyed; ++i) {
            std::this_thread::sleep_for(10ms);
        }
        if (!loopDestroyed) {
            proceed.set_value();
        }
    });
    loop.reset();
    const bool destroyedBeforeTaskFinished = proceedFuture.wait_for(0s) != std::future_status::ready;
    loopDestroyed = true;
    timeout.join();
    EXPECT_TRUE(destroyedBeforeTaskFinished);

    if (destroyedBeforeTaskFinished) {
        proceed.set_value();
    }
    pool->waitForEmpty(tag);
    EXPECT_FALSE(replied);
}

TEST(WeakScheduler, ReplySchedulerDestroyedBeforeTaskStarts) {
    // The task does not run if the scheduler its reply is for is already gone.
    const util::SimpleIdentity tag;
    std::shared_ptr<Scheduler> sequenced = std::make_shared<SequencedScheduler>();
    auto loop = std::make_unique<util::RunLoop>();

    std::promise<void> proceed;
    std::shared_future<void> proceedFuture = proceed.get_future().share();
    sequenced->schedule(tag, [proceedFuture] { proceedFuture.wait(); });

    std::atomic<bool> taskRan{false};
    sequenced->scheduleAndReplyValue(
        tag,
        [&] {
            taskRan = true;
            return 1;
        },
        [](int) {});

    loop.reset();
    proceed.set_value();
    sequenced->waitForEmpty(tag);
    EXPECT_FALSE(taskRan);
}
