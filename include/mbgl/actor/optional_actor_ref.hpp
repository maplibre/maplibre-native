#pragma once

#include <mbgl/actor/actor_ref.hpp>

namespace mbgl {

class TaggedScheduler;

template <class Object>
class OptionalActorRef {
public:
    OptionalActorRef() = default;

    OptionalActorRef(bool syncRef, Object& object, std::weak_ptr<Mailbox> weakMailbox) {
        if (syncRef) {
            objectRef = &object;
        } else {
            actorRef = std::make_optional<ActorRef<Object>>(object, weakMailbox);
        }
    }

    explicit OptionalActorRef(Object& object) { objectRef = &object; }

    explicit OptionalActorRef(ActorRef<Object> actorRef_) { actorRef = actorRef_; }

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
    std::optional<ActorRef<Object>> actorRef;
};

} // namespace mbgl
