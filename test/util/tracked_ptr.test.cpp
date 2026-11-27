#include <mln/test/util.hpp>

#include <mln/util/tracked_ptr.hpp>

#include <memory>

using namespace mln::util;

TEST(TrackedPtr, Empty) {
    TrackedPtr<int> ptr;
    EXPECT_EQ(nullptr, ptr.get());
}

TEST(TrackedPtr, NullOnceTrackingEnds) {
    int object = 0;
    PointerTracker tracker;

    const auto ptr = tracker.track(object);
    const auto copy = ptr;
    EXPECT_EQ(&object, ptr.get());
    EXPECT_EQ(&object, copy.get());

    tracker.end();
    EXPECT_EQ(nullptr, ptr.get());
    EXPECT_EQ(nullptr, copy.get());
    EXPECT_EQ(nullptr, tracker.track(object).get());
}

TEST(TrackedPtr, OutlivesTracker) {
    struct Owner {
        PointerTracker tracker;
    };

    auto owner = std::make_unique<Owner>();
    const auto ptr = owner->tracker.track(*owner);
    EXPECT_EQ(owner.get(), ptr.get());

    owner.reset();
    EXPECT_EQ(nullptr, ptr.get());
}
