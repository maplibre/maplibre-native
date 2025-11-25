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
            object = std::make_unique<Object>(std::forward<Args>(args)...);
        } else {
            actor = std::make_unique<Actor<Object>>(scheduler, std::forward<Args>(args)...);
        }
    }

    template <class... Args>
    OptionalActor(bool syncObject, const TaggedScheduler& scheduler, Args&&... args) {
        if (syncObject) {
            object = std::make_unique<Object>(std::forward<Args>(args)...);
        } else {
            actor = std::make_unique<Actor<Object>>(scheduler, std::forward<Args>(args)...);
        }
    }

    template <class... Args>
    OptionalActor(bool syncObject, std::shared_ptr<Scheduler> scheduler, Args&&... args) {
        if (syncObject) {
            object = std::make_unique<Object>(std::forward<Args>(args)...);
        } else {
            actor = std::make_unique<Actor<Object>>(scheduler, std::forward<Args>(args)...);
        }
    }

    OptionalActor(const OptionalActor&) = delete;

    OptionalActorRef<Object> self() {
        return OptionalActorRef(object, actor ? std::make_unique<ActorRef<Object>>(actor->self()) : nullptr);
    }

private:
    std::unique_ptr<Object> object;
    std::unique_ptr<Actor<Object>> actor;
};

} // namespace mbgl
