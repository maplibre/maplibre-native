#include "mapbox/std/weak.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <ctime>
#include <memory>
#include <thread>
#include <vector>

TEST(WeakPtr, Lock) {
    using namespace std::chrono_literals;
    static std::atomic_int g_i;
    struct TestLock {
        void inc() { ++g_i; }
        mapbox::base::WeakPtrFactory<TestLock> factory_{this};
    };

    auto t = std::make_unique<TestLock>();
    auto weak1 = t->factory_.makeWeakPtr();
    auto weak2 = t->factory_.makeWeakPtr();
    auto weak3 = weak2;

    std::thread thread1([&] {
        auto guard = weak1.lock();
        std::this_thread::sleep_for(150ms);
        weak1->inc();
    });
    std::thread thread2([&] {
        auto guard = weak2.lock();
        std::this_thread::sleep_for(200ms);
        weak2->inc();
    });
    {
        auto guard = weak3.lock();
        std::this_thread::sleep_for(50ms);
        weak3->inc();
    }

    EXPECT_FALSE(weak1.expired());
    EXPECT_FALSE(weak2.expired());
    EXPECT_TRUE(weak1);
    EXPECT_TRUE(weak2);
    ASSERT_NO_THROW(t.reset()); // Should not crash.
    thread1.join();
    thread2.join();

    EXPECT_TRUE(weak1.expired());
    EXPECT_TRUE(weak2.expired());
    EXPECT_FALSE(weak1);
    EXPECT_FALSE(weak2);
    EXPECT_EQ(g_i, 3);
}

TEST(WeakPtr, InvalidateWeakPtrs) {
    using namespace std::chrono_literals;
    static std::atomic_int g_i;
    struct TestLock {
        void inc() { ++g_i; }
        mapbox::base::WeakPtrFactory<TestLock> factory_{this};
    };

    auto t = std::make_unique<TestLock>();
    auto weak1 = t->factory_.makeWeakPtr();
    auto weak2 = t->factory_.makeWeakPtr();
    auto weak3 = weak2;

    std::thread thread1([&] {
        auto guard = weak1.lock();
        std::this_thread::sleep_for(150ms);
        weak1->inc();
    });
    std::thread thread2([&] {
        auto guard = weak2.lock();
        std::this_thread::sleep_for(200ms);
        weak2->inc();
    });
    {
        auto guard = weak3.lock();
        std::this_thread::sleep_for(50ms);
        weak3->inc();
    }

    EXPECT_TRUE(weak1);
    EXPECT_TRUE(weak2);
    EXPECT_TRUE(weak3);
    t->factory_.invalidateWeakPtrs();

    // All the existing weak pointer have expired.
    EXPECT_FALSE(weak3);

    thread1.join();
    thread2.join();

    EXPECT_FALSE(weak1);
    EXPECT_FALSE(weak2);
    EXPECT_EQ(g_i, 3);

    // The newly created one is empty.
    auto weak4 = t->factory_.makeWeakPtr();
    EXPECT_FALSE(weak4);
}

TEST(WeakPtr, MultipleLock) {
    using namespace std::chrono_literals;
    using std::chrono::system_clock;

    static std::atomic_int g_i;
    struct TestLock {
        void inc() { ++g_i; }
        mapbox::base::WeakPtrFactory<TestLock> factory_{this};
    };

    std::time_t now = system_clock::to_time_t(system_clock::now());

    struct std::tm* tm = std::localtime(&now);
    ++tm->tm_sec; // Wait for the next second.
    auto nextSecond = system_clock::from_time_t(mktime(tm));

    auto t = std::make_unique<TestLock>();

    const size_t threadsCount = 50u;

    std::vector<std::thread> threads;
    threads.reserve(threadsCount);

    for (size_t i = 0; i < threadsCount; ++i) {
        std::thread thread([nextSecond, weak = t->factory_.makeWeakPtr()] {
            auto guard = weak.lock();
            std::this_thread::sleep_until(nextSecond);
            weak->inc();
        });
        threads.emplace_back(std::move(thread));
    }

    t.reset();
    for (auto& thread : threads) {
        thread.join();
    }
    EXPECT_EQ(g_i, threadsCount);
}

TEST(WeakPtr, WeakMethod) {
    using namespace std::chrono_literals;
    static std::atomic_int g_i;
    class Test {
    public:
        void increaseGlobal(int delta) { g_i += delta; }
        std::function<void(int)> makeWeakIncreaseGlobal() { return factory_.makeWeakMethod(&Test::increaseGlobal); }

    private:
        mapbox::base::WeakPtrFactory<Test> factory_{this};
    };

    auto t = std::make_unique<Test>();
    std::function<void(int)> weak1 = t->makeWeakIncreaseGlobal();
    std::function<void(int)> weak2 = t->makeWeakIncreaseGlobal();
    std::function<void(int)> weak3 = weak2;

    std::thread thread1([&] { weak1(1); });
    std::thread thread2([&] { weak2(10); });
    std::this_thread::sleep_for(50ms);
    weak3(100);

    t.reset(); // Should not crash.
    // The following calls are ignored.
    weak1(1);
    weak2(2);
    weak3(3);
    thread1.join();
    thread2.join();

    EXPECT_EQ(g_i, 111);
}

TEST(WeakPtr, WeakMethodBlock) {
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace std::chrono;
    using namespace std::chrono_literals;
    static std::atomic_bool g_call_finished{false};
    struct Test {
        void block(decltype(1ms) duration) {
            std::this_thread::sleep_for(duration);
            g_call_finished = true;
        }
        mapbox::base::WeakPtrFactory<Test> factory_{this};
    };

    auto t = std::make_unique<Test>();
    auto weak = t->factory_.makeWeakMethod(&Test::block);
    auto first = high_resolution_clock::now();

    std::thread thread([&] { weak(100ms); });
    std::this_thread::sleep_for(10ms);
    t.reset(); // Deletion is blocked until weak(100ms) call returns.
    thread.join();
    auto totalTime = duration_cast<milliseconds>(high_resolution_clock::now() - first);

    EXPECT_TRUE(g_call_finished);
    EXPECT_GE(totalTime, 100ms);
}
