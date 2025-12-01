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
            createObject(std::forward<Args>(args)...);
        } else {
            actor = std::make_unique<Actor<Object>>(scheduler, std::forward<Args>(args)...);
            selfRef = OptionalActorRef(actor->self());
        }
    }

    template <class... Args>
    OptionalActor(bool syncObject, const TaggedScheduler& scheduler, Args&&... args) {
        if (syncObject) {
            createObject(std::forward<Args>(args)...);
        } else {
            actor = std::make_unique<Actor<Object>>(scheduler, std::forward<Args>(args)...);
            selfRef = OptionalActorRef(actor->self());
        }
    }

    template <class... Args>
    OptionalActor(bool syncObject, std::shared_ptr<Scheduler> scheduler, Args&&... args) {
        if (syncObject) {
            createObject(std::forward<Args>(args)...);
        } else {
            actor = std::make_unique<Actor<Object>>(scheduler, std::forward<Args>(args)...);
            selfRef = OptionalActorRef(actor->self());
        }
    }

    OptionalActor(const OptionalActor&) = delete;
    
    ~OptionalActor() {
        if (object) {
            object->~Object();
            std::free(object);
        }
    }

    OptionalActorRef<Object> self() { return selfRef; }
    
private:
    template <class... Args>
    void createObject(Args&&... args) {
        void* buffer = std::aligned_alloc(alignof(Object), sizeof(Object));
        selfRef = OptionalActorRef(reinterpret_cast<Object&>(*buffer));
        object = new (buffer) Object(selfRef, std::forward<Args>(args)...);
    }
    
    Object* object = nullptr;
    std::unique_ptr<Actor<Object>> actor;
    OptionalActorRef<Object> selfRef;
};

} // namespace mbgl
