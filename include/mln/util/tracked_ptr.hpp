#pragma once

#include <atomic>
#include <memory>

namespace mln {
namespace util {

class PointerTracker;

/// A non-owning pointer that turns null once its object starts being destroyed.
///
/// It does not keep the object alive: on a thread other than the one that destroys the object, the object can start
/// being destroyed right after `get()` returned it.
template <typename T>
class TrackedPtr {
public:
    TrackedPtr() = default;

    /// Returns the object, or null if this pointer is empty or the object is being or has been destroyed.
    T* get() const noexcept { return alive && *alive ? object : nullptr; }

private:
    friend class PointerTracker;

    TrackedPtr(std::shared_ptr<const std::atomic<bool>> alive_, T* object_) noexcept
        : alive(std::move(alive_)),
          object(object_) {}

    std::shared_ptr<const std::atomic<bool>> alive;
    T* object = nullptr;
};

/// Hands out `TrackedPtr`s to the object that owns it, and turns them null when it is destroyed.
class PointerTracker {
public:
    PointerTracker() = default;
    PointerTracker(const PointerTracker&) = delete;
    PointerTracker& operator=(const PointerTracker&) = delete;
    ~PointerTracker() { end(); }

    template <typename T>
    TrackedPtr<T> track(T& object) const {
        return TrackedPtr<T>(alive, &object);
    }

    /// Turns every `TrackedPtr` handed out by this tracker null before the tracker is destroyed. An owner whose
    /// destructor waits for work that reads it on another thread calls this first in its destructor.
    void end() noexcept { *alive = false; }

private:
    std::shared_ptr<std::atomic<bool>> alive = std::make_shared<std::atomic<bool>>(true);
};

} // namespace util
} // namespace mln
