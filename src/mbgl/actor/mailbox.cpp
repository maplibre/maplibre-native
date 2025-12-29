#include <mbgl/actor/mailbox.hpp>
#include <mbgl/actor/message.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/scoped.hpp>

#include <cassert>

namespace mbgl {

Mailbox::Mailbox() = default;

Mailbox::Mailbox(Scheduler& scheduler_)
    : weakScheduler(scheduler_.makeWeakPtr()) {}

Mailbox::Mailbox(const TaggedScheduler& scheduler_)
    : schedulerTag(scheduler_.tag),
      weakScheduler(scheduler_.get()->makeWeakPtr()) {}

void Mailbox::open(const TaggedScheduler& scheduler_) {
    assert(!weakScheduler);
    schedulerTag = scheduler_.tag;
    open(*scheduler_.get());
}

void Mailbox::open(Scheduler& scheduler_) {
    assert(!weakScheduler);

    // As with close(), block until neither receive() nor push() are in
    // progress, and acquire the two mutexes in the same order.
    std::scoped_lock receivingLock(receivingMutex);
    std::scoped_lock pushingLock(pushingMutex);

    if (closed) {
        return;
    }

    weakScheduler = scheduler_.makeWeakPtr();

    if (!queue.empty()) {
        scheduleToRecieve();
    }
}

void Mailbox::close() {
    abandon();

    // Block until neither receive() nor push() are in progress. Two mutexes are
    // used because receive() must not block send(). Of the two, the receiving
    // mutex must be acquired first, because that is the order that an actor
    // will obtain them when it self-sends a message, and consistent lock
    // acquisition order prevents deadlocks. The receiving mutex is recursive to
    // allow a mailbox (and thus the actor) to close itself.
    std::scoped_lock receivingLock(receivingMutex);
    std::scoped_lock pushingLock(pushingMutex);

    closed = true;

    weakScheduler = {};
}

void Mailbox::abandon() {
    auto idleValue = State::Idle;
    while (!state.compare_exchange_strong(idleValue, State::Abandoned)) {
        if (state == State::Abandoned) {
            break;
        }
    }
}

bool Mailbox::isOpen() const {
    return !closed && weakScheduler;
}

void Mailbox::push(std::unique_ptr<Message> message) {
    MLN_TRACE_FUNC();
    auto idleState = State::Idle;
    while (!state.compare_exchange_strong(idleState, State::Processing)) {
        if (state == State::Abandoned) {
            return;
        }
    }

    Scoped activityFlag{[this]() {
        if (state == State::Processing) {
            state = State::Idle;
        }
    }};

    {
        MLN_TRACE_ZONE(push lock);
        std::scoped_lock pushingLock(pushingMutex);

        if (closed) {
            state = State::Abandoned;
            return;
        }

        bool wasEmpty = false;
        {
            MLN_TRACE_ZONE(queue lock);
            std::scoped_lock queueLock(queueMutex);
            wasEmpty = queue.empty();
            queue.push(std::move(message));
        }

        if (wasEmpty) {
            MLN_TRACE_ZONE(schedule);
            scheduleToRecieve(schedulerTag);
        }
    }
}

void Mailbox::receive() {
    auto idleState = State::Idle;
    while (!state.compare_exchange_strong(idleState, State::Processing)) {
        if (state == State::Abandoned) {
            return;
        }
    }

    Scoped activityFlag{[this]() {
        if (state == State::Processing) {
            state = State::Idle;
        }
    }};
    std::scoped_lock receivingLock(receivingMutex);

    if (closed) {
        state = State::Abandoned;
        return;
    }

    std::unique_ptr<Message> message;
    bool wasEmpty = false;

    {
        std::scoped_lock queueLock(queueMutex);
        assert(!queue.empty());
        message = std::move(queue.front());
        queue.pop();
        wasEmpty = queue.empty();
    }

    (*message)();

    // If there are more messages in the queue and the scheduler
    // is still active, create a new task to handle the next one
    if (!wasEmpty) {
        scheduleToRecieve();
    }
}

void Mailbox::scheduleToRecieve(const std::optional<util::SimpleIdentity>& tag) {
    if (auto guard = weakScheduler.lock(); weakScheduler) {
        std::weak_ptr<Mailbox> mailbox = shared_from_this();
        auto setToRecieve = [mbox = std::move(mailbox)]() {
            if (auto locked = mbox.lock()) {
                locked->receive();
            }
        };
        if (tag) {
            weakScheduler->schedule(*tag, std::move(setToRecieve));
        } else {
            weakScheduler->schedule(std::move(setToRecieve));
        }
    }
}

} // namespace mbgl
