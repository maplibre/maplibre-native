#pragma once

#include <cassert>
#include <memory>
#include <mutex>
#include <utility>

namespace mln {
namespace util {

/// Wraps callbacks so that they stop running once the scope is closed.
///
/// The owner of the scope calls `close()` first in its destructor. `close()` waits for a bound callback that is running
/// on another thread, so no bound callback uses the owner after that. A bound callback may close its own scope, for
/// example by destroying the owner.
class CallbackScope {
public:
    CallbackScope() = default;
    CallbackScope(const CallbackScope&) = delete;
    CallbackScope& operator=(const CallbackScope&) = delete;

    ~CallbackScope() {
        std::scoped_lock lock(state->mutex);
        assert(state->closed && "Call close() first in the owner's destructor");
        state->closed = true;
    }

    /// Returns a callable that calls `fn` with its arguments until the scope is closed, and does nothing after that.
    template <typename Fn>
    auto bind(Fn&& fn) const {
        return [shared = state, callback = std::forward<Fn>(fn)](auto&&... args) {
            std::scoped_lock lock(shared->mutex);
            if (!shared->closed) {
                callback(std::forward<decltype(args)>(args)...);
            }
        };
    }

    void close() {
        std::scoped_lock lock(state->mutex);
        state->closed = true;
    }

private:
    struct State {
        std::recursive_mutex mutex;
        bool closed = false;
    };

    std::shared_ptr<State> state = std::make_shared<State>();
};

} // namespace util
} // namespace mln
