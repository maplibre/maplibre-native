#pragma once

#include <mbgl/actor/actor_ref.hpp>

namespace mbgl {

class TaggedScheduler;

template <class Object>
class OptionalActorRef {
public:
    OptionalActorRef(bool syncRef, Object& object, std::weak_ptr<Mailbox> weakMailbox) {
        if (syncRef) {
            objectRef = &object;
        } else {
            actorRef = std::make_unique<ActorRef<Object>>(object, weakMailbox);
        }
    }

    OptionalActorRef(const std::unique_ptr<Object>& object, std::unique_ptr<ActorRef<Object>> actorRef_) {
        objectRef = object.get();
        actorRef = std::move(actorRef_);
    }

    OptionalActorRef(const OptionalActorRef&) = delete;

    template <typename Fn, class... Args>
    void invoke(Fn fn, Args&&... args) const {
        if (objectRef) {
            (objectRef->*fn)(std::forward<Args>(args)...);
        } else if (actorRef) {
            actorRef->invoke(fn, std::forward<Args>(args)...);
        }
    }

private:
    Object* objectRef = nullptr;
    std::unique_ptr<ActorRef<Object>> actorRef;
};

} // namespace mbgl
