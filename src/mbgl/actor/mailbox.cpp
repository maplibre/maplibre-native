#include <mbgl/actor/mailbox.hpp>
#include <mbgl/actor/message.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/scoped.hpp>

#include <cassert>

namespace mbgl {

Mailbox::Mailbox() = default;

Mailbox::Mailbox(Scheduler& scheduler_)
    : weakScheduler(scheduler_.makeWeakPtr()) {}

void Mailbox::open(Scheduler& scheduler_) {
    assert(!weakScheduler);

    // As with close(), block until neither receive() nor push() are in
    // progress, and acquire the two mutexes in the same order.
    std::lock_guard<std::recursive_mutex> receivingLock(receivingMutex);
    std::lock_guard<std::mutex> pushingLock(pushingMutex);

    if (closed) {
        return;
    }

    weakScheduler = scheduler_.makeWeakPtr();

    if (!queue.empty()) {
        auto guard = weakScheduler.lock();
        if (weakScheduler) {
            weakScheduler->schedule(makeClosure(shared_from_this()));
        }
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
    std::lock_guard<std::recursive_mutex> receivingLock(receivingMutex);
    std::lock_guard<std::mutex> pushingLock(pushingMutex);

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
    return bool(weakScheduler) && !closed;
}

void Mailbox::push(std::unique_ptr<Message> message) {
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

    std::lock_guard<std::mutex> pushingLock(pushingMutex);

    if (closed) {
        state = State::Abandoned;
        return;
    }

    std::lock_guard<std::mutex> queueLock(queueMutex);
    bool wasEmpty = queue.empty();
    queue.push(std::move(message));
    auto guard = weakScheduler.lock();
    if (wasEmpty && weakScheduler) {
        weakScheduler->schedule(makeClosure(shared_from_this()));
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
    std::lock_guard<std::recursive_mutex> receivingLock(receivingMutex);

    if (closed) {
        state = State::Abandoned;
        return;
    }

    std::unique_ptr<Message> message;
    bool wasEmpty;

    {
        std::lock_guard<std::mutex> queueLock(queueMutex);
        assert(!queue.empty());
        message = std::move(queue.front());
        queue.pop();
        wasEmpty = queue.empty();
    }

    (*message)();

    // If there are more messages in the queue and the scheduler
    // is still active, create a new task to handle the next one
    if (!wasEmpty) {
        auto guard = weakScheduler.lock();
        if (weakScheduler) {
            weakScheduler->schedule(makeClosure(shared_from_this()));
        }
    }
}

// static
void Mailbox::maybeReceive(const std::weak_ptr<Mailbox>& mailbox) {
    if (auto locked = mailbox.lock()) {
        locked->receive();
    }
}

// static
std::function<void()> Mailbox::makeClosure(std::weak_ptr<Mailbox> mailbox) {
    return [mailbox = std::move(mailbox)]() {
        maybeReceive(mailbox);
    };
}

} // namespace mbgl
