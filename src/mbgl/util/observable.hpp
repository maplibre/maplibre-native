#pragma once

#include <map>
#include <mapbox/compatibility/value.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/chrono.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace mbgl {

class Observer;

struct ObservableEvent {
    explicit ObservableEvent(std::string type);
    std::string type;
    TimePoint begin;
    TimePoint end;
    mapbox::base::Value data;
};

class Observer {
public:
    virtual ~Observer() = default;

    // Notifies Observer about an event
    virtual void notify(const ObservableEvent&) = 0;

    // Id of an observer, must be unique.
    // Default implementation uses `this` address of an instance.
    virtual std::size_t id() const;
};

// Observable class implements basic Publish&Subscribe functionality
class Observable {
public:
    Observable();
    virtual ~Observable();

    // Subscribes `Observer` to recieve notifications of the event types provided in `eventTypes`
    // `Observer::id()` would be used as an `id` for subscription.
    virtual void subscribe(const std::shared_ptr<Observer>&, const std::vector<std::string>& eventTypes);

    // Unsubscribes perviosly subscribed `Observer` with an `id`, that is provided by `Observer::id()`
    // Whenever an empty list of events is provided, observer would be unsubscribed from all events.
    virtual void unsubscribeWithId(std::size_t id, const std::vector<std::string>& eventTypes);

    // Must return true if `Observable` supports an `eventType`.
    virtual bool supportsEventType(const std::string& eventType) const;

    // Shorthand for virtual method accepting `Observer` id.
    void unsubscribe(const std::shared_ptr<Observer>&, const std::vector<std::string>& eventTypes);
    void unsubscribe(const std::shared_ptr<Observer>&);

protected:
    void dispatchEvent(const ObservableEvent&);
    bool hasSubscribers() const;

private:
    struct Subscriber;
    std::map<std::size_t, std::unique_ptr<Subscriber>> subscribers;
    mutable std::mutex mutex;
};

} // namespace mbgl
