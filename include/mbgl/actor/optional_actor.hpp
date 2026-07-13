#pragma once

#include <mbgl/actor/actor.hpp>
#include <mbgl/actor/optional_actor_ref.hpp>

namespace mbgl {

class TaggedScheduler;

template <class Object>
class OptionalActor {
public:
    template <class... Args>
    OptionalActor(bool syncObject, Scheduler& scheduler, Args&&... args) {
        if (syncObject) {
            object = std::make_unique<SyncObject>(std::forward<Args>(args)...);
            selfRef = OptionalActorRef(object->self());
        } else {
            actor = std::make_unique<Actor<Object>>(scheduler, std::forward<Args>(args)...);
            selfRef = OptionalActorRef(actor->self());
        }
    }

    template <class... Args>
    OptionalActor(bool syncObject, const TaggedScheduler& scheduler, Args&&... args) {
        if (syncObject) {
            object = std::make_unique<SyncObject>(std::forward<Args>(args)...);
            selfRef = OptionalActorRef(object->self());
        } else {
            actor = std::make_unique<Actor<Object>>(scheduler, std::forward<Args>(args)...);
            selfRef = OptionalActorRef(actor->self());
        }
    }

    template <class... Args>
    OptionalActor(bool syncObject, std::shared_ptr<Scheduler> scheduler, Args&&... args) {
        if (syncObject) {
            object = std::make_unique<SyncObject>(std::forward<Args>(args)...);
            selfRef = OptionalActorRef(object->self());
        } else {
            actor = std::make_unique<Actor<Object>>(scheduler, std::forward<Args>(args)...);
            selfRef = OptionalActorRef(actor->self());
        }
    }

    OptionalActor(const OptionalActor&) = delete;

    const OptionalActorRef<Object>& self() { return selfRef; }

private:
    class SyncObject {
    public:
        template <class... Args>
        SyncObject(Args&&... args)
            requires(std::is_constructible_v<Object, OptionalActorRef<Object>, Args...>)
        {
            new (&objectStorage) Object(OptionalActorRef(self()), std::forward<Args>(args)...);
        }

        template <class... Args>
        SyncObject(Args&&... args)
            requires(std::is_constructible_v<Object, Args...>)
        {
            new (&objectStorage) Object(std::forward<Args>(args)...);
        }

        SyncObject(const SyncObject&) = delete;

        ~SyncObject() { self().~Object(); }

        Object& self() { return reinterpret_cast<Object&>(objectStorage); }

    private:
        std::aligned_storage_t<sizeof(Object)> objectStorage;
    };

    std::unique_ptr<SyncObject> object;
    std::unique_ptr<Actor<Object>> actor;
    OptionalActorRef<Object> selfRef;
};

} // namespace mbgl
