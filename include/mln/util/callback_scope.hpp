#pragma once

#include <memory>
#include <mutex>
#include <utility>

namespace mln {
namespace util {

/// Wraps callbacks so that they stop running once the scope is closed or destroyed.
///
/// Closing waits for a bound callback that is running on another thread, and a bound callback may close its own scope,
/// for example by destroying its owner.
class CallbackScope {
public:
    CallbackScope() = default;
    CallbackScope(const CallbackScope&) = delete;
    CallbackScope& operator=(const CallbackScope&) = delete;
    ~CallbackScope() { close(); }

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

    /// Stops the bound callbacks before the scope is destroyed. An owner whose callbacks can run on another thread
    /// while it is destroyed calls this first in its destructor, so that they stop before its members are destroyed.
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
