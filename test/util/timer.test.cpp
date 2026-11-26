#include <mln/test/util.hpp>

#include <mln/util/chrono.hpp>
#include <mln/util/monotonic_timer.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/string.hpp>
#include <mln/util/timer.hpp>

#include <memory>
#include <optional>

using namespace mln::util;

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(Basic)) {
    RunLoop loop;

    const auto interval = mln::Milliseconds(300);
    const auto expectedTotalTime = interval;

    const auto first = MonotonicTimer::now();
    const auto elapsed = [=] {
        return std::chrono::duration_cast<mln::Milliseconds>(MonotonicTimer::now() - first);
    };
    std::optional<mln::Milliseconds> callbackTime;
    auto callback = [&] {
        callbackTime = elapsed();
        loop.stop();
    };

    Timer timer;
    timer.start(interval, mln::Duration::zero(), std::move(callback));

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

    auto interval = mln::Milliseconds(50);
    auto expectedTotalTime = interval * count;

    auto first = mln::Clock::now();
    timer.start(interval, interval, callback);

    loop.run();

    auto totalTime = std::chrono::duration_cast<mln::Milliseconds>(mln::Clock::now() - first);

    EXPECT_GE(totalTime, expectedTotalTime * 0.8);
    EXPECT_LE(totalTime, expectedTotalTime * 1.3);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(Stop)) {
    RunLoop loop;

    Timer timer1;
    Timer timer2;

    auto interval1 = mln::Milliseconds(50);
    auto interval2 = mln::Milliseconds(250);
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

    auto first = mln::Clock::now();
    timer1.start(interval1, interval1, callback1);
    timer2.start(interval2, mln::Duration::zero(), callback2);

    loop.run();

    auto totalTime = std::chrono::duration_cast<mln::Milliseconds>(mln::Clock::now() - first);

    EXPECT_EQ(count, 2);

    EXPECT_GE(totalTime, expectedTotalTime * 0.8);
    EXPECT_LE(totalTime, expectedTotalTime * 1.2);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(DestroyShouldStop)) {
    RunLoop loop;

    auto timer1 = std::make_unique<Timer>();
    Timer timer2;

    auto interval1 = mln::Milliseconds(50);
    auto interval2 = mln::Milliseconds(250);
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

    auto first = mln::Clock::now();
    timer1->start(interval1, interval1, callback1);
    timer2.start(interval2, mln::Duration::zero(), callback2);

    loop.run();

    auto totalTime = std::chrono::duration_cast<mln::Milliseconds>(mln::Clock::now() - first);

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
    auto expireTimeout = mln::Milliseconds(50);

    auto timerCallback = [&] {
        // we cannot expect much here as in some cases timer may be finished earlier
        // than the loop stop timer (and thus the callback will be called)
    };

    auto loopStopTimerCallback = [&] {
        timer->stop();
        loop.stop();
    };

    auto first = mln::Clock::now();

    loopStopTimer->start(expireTimeout, mln::Milliseconds(0), loopStopTimerCallback);
    timer->start(expireTimeout, mln::Milliseconds(0), timerCallback);

    loop.run();

    auto totalTime = std::chrono::duration_cast<mln::Milliseconds>(mln::Clock::now() - first);

    EXPECT_GE(totalTime, expireTimeout * 0.8);
    EXPECT_LE(totalTime, expireTimeout * 1.3);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(StoppedAfterExpiration)) {
    RunLoop loop;

    auto timer = std::make_unique<Timer>();
    auto loopStopTimer = std::make_unique<Timer>();
    auto expireTimeout = mln::Milliseconds(50);

    bool callbackFired = false;

    auto timerCallback = [&] {
        callbackFired = true;
    };

    auto first = mln::Clock::now();

    timer->start(expireTimeout, mln::Milliseconds(0), timerCallback);

    // poll until the timer expires
    auto expireWaitInterval = expireTimeout * 2;
    auto startWaitTime = mln::Clock::now();
    auto waitDuration = mln::Duration::zero();
    while (waitDuration < expireWaitInterval) {
        waitDuration = std::chrono::duration_cast<mln::Milliseconds>(mln::Clock::now() - startWaitTime);
    }
    timer->stop();

    loop.runOnce();

    auto totalTime = std::chrono::duration_cast<mln::Milliseconds>(mln::Clock::now() - first);

    EXPECT_TRUE(!callbackFired);
    EXPECT_GE(totalTime, expireWaitInterval * 0.8);
    EXPECT_LE(totalTime, expireWaitInterval * 1.2);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(StartOverrides)) {
    RunLoop loop;

    Timer timer;

    auto interval1 = mln::Milliseconds(50);
    auto interval2 = mln::Milliseconds(250);
    auto expectedTotalTime = interval1 + interval2;

    int count = 0;

    auto callback2 = [&] {
        ++count;
        loop.stop();
    };

    auto callback1 = [&] {
        ++count;
        timer.start(interval2, mln::Duration::zero(), callback2);
    };

    auto first = mln::Clock::now();
    timer.start(interval1, mln::Duration::zero(), callback1);

    loop.run();

    auto totalTime = std::chrono::duration_cast<mln::Milliseconds>(mln::Clock::now() - first);

    EXPECT_EQ(count, 2);

    EXPECT_GE(totalTime, expectedTotalTime * 0.8);
    EXPECT_LE(totalTime, expectedTotalTime * 1.2);
}

TEST(Timer, TEST_REQUIRES_ACCURATE_TIMING(CanStopNonStartedTimer)) {
    RunLoop loop;

    Timer timer;
    timer.stop();
}
