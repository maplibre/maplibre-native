#include <mbgl/test/util.hpp>

#include <mbgl/util/chrono.hpp>
#include <mbgl/util/monotonic_timer.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/timer.hpp>

#include <memory>
#include <optional>

using namespace mbgl::util;

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(Basic)) {
    RunLoop loop;

    const auto interval = mbgl::Milliseconds(300);
    const auto expectedTotalTime = interval;

    const auto first = MonotonicTimer::now();
    const auto elapsed = [=] {
        return std::chrono::duration_cast<mbgl::Milliseconds>(MonotonicTimer::now() - first);
    };
    std::optional<mbgl::Milliseconds> callbackTime;
    auto callback = [&] {
        callbackTime = elapsed();
        loop.stop();
    };

    Timer timer;
    timer.start(interval, mbgl::Duration::zero(), std::move(callback));

    loop.run();

    const auto totalTime = elapsed();

    SCOPED_TRACE("Timer callback: " + (callbackTime ? toString(callbackTime->count()) : "(never)"));

    EXPECT_GE(totalTime.count(), (expectedTotalTime * 0.99).count());
    EXPECT_LE(totalTime.count(), (expectedTotalTime * 1.10).count());
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(Repeat)) {
    RunLoop loop;

    Timer timer;

    unsigned count = 10;
    auto callback = [&] {
        if (!--count) {
            loop.stop();
        }
    };

    auto interval = mbgl::Milliseconds(50);
    auto expectedTotalTime = interval * count;

    auto first = mbgl::Clock::now();
    timer.start(interval, interval, callback);

    loop.run();

    auto totalTime = std::chrono::duration_cast<mbgl::Milliseconds>(mbgl::Clock::now() - first);

    EXPECT_GE(totalTime, expectedTotalTime * 0.8);
    EXPECT_LE(totalTime, expectedTotalTime * 1.3);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(Stop)) {
    RunLoop loop;

    Timer timer1;
    Timer timer2;

    auto interval1 = mbgl::Milliseconds(50);
    auto interval2 = mbgl::Milliseconds(250);
    auto expectedTotalTime = interval2;

    int count = 0;

    auto callback1 = [&] {
        ++count;
        timer1.stop();
    };

    auto callback2 = [&] {
        ++count;
        loop.stop();
    };

    auto first = mbgl::Clock::now();
    timer1.start(interval1, interval1, callback1);
    timer2.start(interval2, mbgl::Duration::zero(), callback2);

    loop.run();

    auto totalTime = std::chrono::duration_cast<mbgl::Milliseconds>(mbgl::Clock::now() - first);

    EXPECT_EQ(count, 2);

    EXPECT_GE(totalTime, expectedTotalTime * 0.8);
    EXPECT_LE(totalTime, expectedTotalTime * 1.2);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(DestroyShouldStop)) {
    RunLoop loop;

    auto timer1 = std::make_unique<Timer>();
    Timer timer2;

    auto interval1 = mbgl::Milliseconds(50);
    auto interval2 = mbgl::Milliseconds(250);
    auto expectedTotalTime = interval2;

    int count = 0;

    auto callback1 = [&] {
        ++count;
        timer1.reset();
    };

    auto callback2 = [&] {
        ++count;
        loop.stop();
    };

    auto first = mbgl::Clock::now();
    timer1->start(interval1, interval1, callback1);
    timer2.start(interval2, mbgl::Duration::zero(), callback2);

    loop.run();

    auto totalTime = std::chrono::duration_cast<mbgl::Milliseconds>(mbgl::Clock::now() - first);

    EXPECT_EQ(count, 2);

    EXPECT_GE(totalTime, expectedTotalTime * 0.8);
    EXPECT_LE(totalTime, expectedTotalTime * 1.2);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(StoppedDuringExpiration)) {
    // The idea is to have original timer cancellation and expiration roughly at
    // the same time. In this case some timer backens (e.g.
    // asio::high_resolution_timer) may call the expiration callback with good
    // status while the timer may not expect it.

    RunLoop loop;

    auto timer = std::make_unique<Timer>();
    auto loopStopTimer = std::make_unique<Timer>();
    auto expireTimeout = mbgl::Milliseconds(50);

    auto timerCallback = [&] {
        // we cannot expect much here as in some cases timer may be finished earlier
        // than the loop stop timer (and thus the callback will be called)
    };

    auto loopStopTimerCallback = [&] {
        timer->stop();
        loop.stop();
    };

    auto first = mbgl::Clock::now();

    loopStopTimer->start(expireTimeout, mbgl::Milliseconds(0), loopStopTimerCallback);
    timer->start(expireTimeout, mbgl::Milliseconds(0), timerCallback);

    loop.run();

    auto totalTime = std::chrono::duration_cast<mbgl::Milliseconds>(mbgl::Clock::now() - first);

    EXPECT_GE(totalTime, expireTimeout * 0.8);
    EXPECT_LE(totalTime, expireTimeout * 1.3);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(StoppedAfterExpiration)) {
    RunLoop loop;

    auto timer = std::make_unique<Timer>();
    auto loopStopTimer = std::make_unique<Timer>();
    auto expireTimeout = mbgl::Milliseconds(50);

    bool callbackFired = false;

    auto timerCallback = [&] {
        callbackFired = true;
    };

    auto first = mbgl::Clock::now();

    timer->start(expireTimeout, mbgl::Milliseconds(0), timerCallback);

    // poll until the timer expires
    auto expireWaitInterval = expireTimeout * 2;
    auto startWaitTime = mbgl::Clock::now();
    auto waitDuration = mbgl::Duration::zero();
    while (waitDuration < expireWaitInterval) {
        waitDuration = std::chrono::duration_cast<mbgl::Milliseconds>(mbgl::Clock::now() - startWaitTime);
    }
    timer->stop();

    loop.runOnce();

    auto totalTime = std::chrono::duration_cast<mbgl::Milliseconds>(mbgl::Clock::now() - first);

    EXPECT_TRUE(!callbackFired);
    EXPECT_GE(totalTime, expireWaitInterval * 0.8);
    EXPECT_LE(totalTime, expireWaitInterval * 1.2);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(StartOverrides)) {
    RunLoop loop;

    Timer timer;

    auto interval1 = mbgl::Milliseconds(50);
    auto interval2 = mbgl::Milliseconds(250);
    auto expectedTotalTime = interval1 + interval2;

    int count = 0;

    auto callback2 = [&] {
        ++count;
        loop.stop();
    };

    auto callback1 = [&] {
        ++count;
        timer.start(interval2, mbgl::Duration::zero(), callback2);
    };

    auto first = mbgl::Clock::now();
    timer.start(interval1, mbgl::Duration::zero(), callback1);

    loop.run();

    auto totalTime = std::chrono::duration_cast<mbgl::Milliseconds>(mbgl::Clock::now() - first);

    EXPECT_EQ(count, 2);

    EXPECT_GE(totalTime, expectedTotalTime * 0.8);
    EXPECT_LE(totalTime, expectedTotalTime * 1.2);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(CanStopNonStartedTimer)) {
    RunLoop loop;

    Timer timer;
    timer.stop();
}
