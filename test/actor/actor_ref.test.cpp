#include <mbgl/actor/actor.hpp>

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/test/util.hpp>

#include <future>

using namespace mbgl;

TEST(ActorRef, CanOutliveActor) {
    // An ActorRef can outlive its actor. Doing does not extend the actor's lifetime.
    // Sending a message to an ActorRef whose actor has died is a no-op.

    struct TestActorRef {
        bool& died;

        TestActorRef(ActorRef<TestActorRef>, bool& died_)
            : died(died_) {}

        ~TestActorRef() { died = true; }

        void receive() { FAIL(); }
    };

    bool died = false;

    ActorRef<TestActorRef> test = [&]() {
        return Actor<TestActorRef>(Scheduler::GetBackground(), std::ref(died)).self();
    }();

    EXPECT_TRUE(died);
    test.invoke(&TestActorRef::receive);
}

TEST(ActorRef, Ask) {
    // Ask returns a Future eventually returning the result

    struct TestActorRef {
        TestActorRef(ActorRef<TestActorRef>) {}

        int gimme() { return 20; }

        int echo(int i) { return i; }
    };

    Actor<TestActorRef> actor(Scheduler::GetBackground());
    ActorRef<TestActorRef> ref = actor.self();

    EXPECT_EQ(20, ref.ask(&TestActorRef::gimme).get());
    EXPECT_EQ(30, ref.ask(&TestActorRef::echo, 30).get());
}

TEST(ActorRef, AskVoid) {
    // Ask waits for void methods

    struct TestActorRef {
        bool& executed;

        TestActorRef(bool& executed_)
            : executed(executed_) {}

        void doIt() { executed = true; }
    };

    bool executed = false;
    Actor<TestActorRef> actor(Scheduler::GetBackground(), executed);
    ActorRef<TestActorRef> ref = actor.self();

    ref.ask(&TestActorRef::doIt).get();
    EXPECT_TRUE(executed);
}

TEST(ActorRef, AskOnDestroyedActor) {
    // Tests behavior when calling ask() after the
    // Actor has gone away. Should set a exception_ptr.

    struct TestActorRef {
        bool& died;

        TestActorRef(ActorRef<TestActorRef>, bool& died_)
            : died(died_) {}

        ~TestActorRef() { died = true; }

        int receive() { return 1; }
    };
    bool died = false;

    auto actor = std::make_unique<Actor<TestActorRef>>(Scheduler::GetBackground(), died);
    ActorRef<TestActorRef> ref = actor->self();

    actor.reset();
    EXPECT_TRUE(died);

    // the code below does not compile with newer versions of gcc due to
    // (correctly) identifying a -Werror=use-after-free situation
    // the problem is that the ActorRef has a pointer to the actor
    // whose memory is managed by AspiringActor, which has been destroyed
    // mitigation would require getting rid of the raw pointer
    // See https://github.com/maplibre/maplibre-native/issues/876

    // auto result = ref.ask(&TestActorRef::receive);
    // EXPECT_ANY_THROW(result.get());
}
