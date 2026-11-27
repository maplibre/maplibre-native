#include <mln/test/util.hpp>

#include <mln/util/callback_scope.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <thread>

using namespace mln::util;
using namespace std::chrono_literals;

TEST(CallbackScope, CallsUntilClosed) {
    CallbackScope scope;
    int calls = 0;
    const auto callback = scope.bind([&](int value) { calls += value; });

    callback(1);
    callback(2);
    EXPECT_EQ(3, calls);

    scope.close();
    callback(4);
    EXPECT_EQ(3, calls);
}

TEST(CallbackScope, ClosesWhenDestroyed) {
    auto scope = std::make_unique<CallbackScope>();
    int calls = 0;
    const auto callback = scope->bind([&] { ++calls; });

    scope.reset();
    callback();
    EXPECT_EQ(0, calls);
}

TEST(CallbackScope, ForwardsArguments) {
    struct CopyCounter {
        explicit CopyCounter(int& copies_)
            : copies(copies_) {}
        CopyCounter(const CopyCounter& other)
            : copies(other.copies) {
            ++copies;
        }
        CopyCounter(CopyCounter&&) = default;
        int& copies;
    };

    CallbackScope scope;
    int copies = 0;
    const auto callback = scope.bind([](CopyCounter) {});

    callback(CopyCounter(copies));
    EXPECT_EQ(0, copies);

    const CopyCounter counter(copies);
    callback(counter);
    EXPECT_EQ(1, copies);
}

TEST(CallbackScope, CloseWaitsForCallbackOnAnotherThread) {
    CallbackScope scope;
    std::promise<void> entered;
    std::atomic<bool> finished{false};
    const auto callback = scope.bind([&] {
        entered.set_value();
        std::this_thread::sleep_for(100ms);
        finished = true;
    });

    std::thread thread(callback);
    entered.get_future().wait();
    scope.close();
    EXPECT_TRUE(finished);
    thread.join();
}

TEST(CallbackScope, CallbackDestroysItsOwner) {
    struct Owner {
        CallbackScope callbacks;
    };

    auto owner = std::make_unique<Owner>();
    int calls = 0;
    const auto callback = owner->callbacks.bind([&] {
        ++calls;
        owner.reset();
    });

    callback();
    EXPECT_EQ(nullptr, owner);
    callback();
    EXPECT_EQ(1, calls);
}
