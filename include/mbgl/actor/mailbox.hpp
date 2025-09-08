#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>

#include <mapbox/std/weak.hpp>
#include <mbgl/actor/scheduler.hpp>

namespace mbgl {

class Scheduler;
class Message;

class Mailbox : public std::enable_shared_from_this<Mailbox> {
public:
    /// Create a "holding" mailbox, messages to which will remain queued,
    /// unconsumed, until the mailbox is associated with a Scheduler using
    /// start(). This allows a Mailbox object to be created on one thread and
    /// later transferred to a different target thread that may not yet exist.
    Mailbox();

    Mailbox(Scheduler&);
    Mailbox(const TaggedScheduler&);

    /// Attach the given scheduler to this mailbox and begin processing messages
    /// sent to it. The mailbox must be a "holding" mailbox, as created by the
    /// default constructor Mailbox().
    void open(const TaggedScheduler& scheduler_);
    void open(Scheduler&);
    void close();

    // Indicate this mailbox will no longer be checked for messages
    void abandon();

    bool isOpen() const;

    void push(std::unique_ptr<Message>);
    void receive();

private:
    void scheduleToRecieve(const std::optional<util::SimpleIdentity>& tag = std::nullopt);
    enum class State : uint32_t {
        Idle = 0,
        Processing,
        Abandoned
    };

    util::SimpleIdentity schedulerTag = util::SimpleIdentity::Empty;
    mapbox::base::WeakPtr<Scheduler> weakScheduler;

    std::recursive_mutex receivingMutex;
    std::mutex pushingMutex;

    std::atomic<State> state{State::Idle};
    bool closed{false};

    std::mutex queueMutex;
    std::queue<std::unique_ptr<Message>> queue;
};

} // namespace mbgl
