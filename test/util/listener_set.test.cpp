#include <gtest/gtest.h>

#include <mbgl/util/listener_set.hpp>

#include <string>
#include <vector>

using namespace mbgl;

// ListenerSet is a minimal handle-based fan-out container used by
// RenderRasterDEMSource (and similar) to publish per-event callbacks to
// multiple subscribers. Pure-C++; no MapLibre deps; testable in isolation.

TEST(ListenerSet, EmptyByDefault) {
    ListenerSet<int> set;
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0u);
}

TEST(ListenerSet, NotifyFiresAllListeners) {
    ListenerSet<int> set;
    std::vector<int> seenA;
    std::vector<int> seenB;
    set.add([&](int v) { seenA.push_back(v); });
    set.add([&](int v) { seenB.push_back(v); });

    set.notify(7);
    set.notify(42);

    EXPECT_EQ(seenA, (std::vector<int>{7, 42}));
    EXPECT_EQ(seenB, (std::vector<int>{7, 42}));
}

TEST(ListenerSet, NotifyOnEmptySetIsNoOp) {
    ListenerSet<int> set;
    set.notify(1); // must not crash
    SUCCEED();
}

TEST(ListenerSet, RemovedListenerNoLongerFires) {
    ListenerSet<int> set;
    int callsA = 0;
    int callsB = 0;
    auto handleA = set.add([&](int) { ++callsA; });
    set.add([&](int) { ++callsB; });

    set.notify(1);
    EXPECT_EQ(callsA, 1);
    EXPECT_EQ(callsB, 1);

    set.remove(handleA);
    set.notify(2);
    EXPECT_EQ(callsA, 1) << "removed listener should not fire again";
    EXPECT_EQ(callsB, 2);
}

TEST(ListenerSet, RemoveUnknownHandleIsHarmless) {
    ListenerSet<int> set;
    set.remove(999); // never registered — must not crash
    SUCCEED();
}

TEST(ListenerSet, HandlesAreUnique) {
    // Sanity: removing one listener doesn't disturb others sharing similar
    // construction. Two listeners → two distinct handles.
    ListenerSet<int> set;
    auto h1 = set.add([](int) {});
    auto h2 = set.add([](int) {});
    EXPECT_NE(h1, h2);
}

TEST(ListenerSet, SupportsByValueAndByReferenceArgs) {
    // ListenerSet<T&> notifies with a reference; commonly used to pass a
    // const tile object to all subscribers without copying.
    ListenerSet<const std::string&> set;
    std::string captured;
    set.add([&](const std::string& s) { captured = s; });

    const std::string sent = "hello";
    set.notify(sent);
    EXPECT_EQ(captured, "hello");
}

TEST(ListenerSet, ListenerCanAddAnotherDuringNotify) {
    // Reentrancy: the snapshot-then-iterate pattern in notify() lets a
    // listener register a new listener without invalidating the in-flight
    // iteration. The freshly-added listener does NOT see the current event
    // (it's added after the snapshot) but does see subsequent ones.
    ListenerSet<int> set;
    int adderCalls = 0;
    int newListenerCalls = 0;
    set.add([&](int) {
        ++adderCalls;
        if (adderCalls == 1) {
            set.add([&](int) { ++newListenerCalls; });
        }
    });

    set.notify(1);
    EXPECT_EQ(adderCalls, 1);
    EXPECT_EQ(newListenerCalls, 0) << "listener added mid-notify must not see the current event";

    set.notify(2);
    EXPECT_EQ(adderCalls, 2);
    EXPECT_EQ(newListenerCalls, 1) << "listener added during the previous notify must see subsequent events";
}

TEST(ListenerSet, ListenerCanRemoveItselfDuringNotify) {
    // Reentrancy: a listener may unregister itself mid-iteration without
    // invalidating the in-flight notify(). Because notify() iterates a
    // snapshot of the listener map, the self-removed listener still fires
    // exactly once for the current event.
    ListenerSet<int> set;
    int selfRemoverCalls = 0;
    int otherCalls = 0;
    ListenerSet<int>::Handle selfHandle{};
    selfHandle = set.add([&](int) {
        ++selfRemoverCalls;
        set.remove(selfHandle);
    });
    set.add([&](int) { ++otherCalls; });

    set.notify(1);
    EXPECT_EQ(selfRemoverCalls, 1);
    EXPECT_EQ(otherCalls, 1);

    set.notify(2);
    EXPECT_EQ(selfRemoverCalls, 1) << "self-removed listener should not fire again";
    EXPECT_EQ(otherCalls, 2);
}

TEST(ListenerSet, ListenerCanRemoveSiblingDuringNotify) {
    // The sibling is removed mid-notify. Because we iterate a snapshot, the
    // sibling still fires once for THIS notify (it was present when the
    // snapshot was taken) and stops firing for subsequent events.
    ListenerSet<int> set;
    int removerCalls = 0;
    int siblingCalls = 0;
    ListenerSet<int>::Handle siblingHandle{};
    set.add([&](int) {
        ++removerCalls;
        set.remove(siblingHandle);
    });
    siblingHandle = set.add([&](int) { ++siblingCalls; });

    set.notify(1);
    EXPECT_EQ(removerCalls, 1);
    EXPECT_EQ(siblingCalls, 1) << "sibling present at snapshot time fires once for the in-flight notify";

    set.notify(2);
    EXPECT_EQ(removerCalls, 2);
    EXPECT_EQ(siblingCalls, 1) << "sibling removed during prior notify should not fire again";
}
