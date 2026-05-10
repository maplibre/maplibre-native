#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <utility>

namespace mbgl {

// Minimal handle-based fan-out container. `add()` registers a callback and
// returns a handle that uniquely identifies the registration; `remove()`
// unregisters by handle; `notify()` fans out to every registered listener.
// Used by RenderRasterDEMSource (and similar) to publish per-tile-load
// events to multiple cross-source consumers without coupling them to each
// other.
//
// Reentrancy: `notify()` snapshots the listener map before iterating so a
// listener may call `add()` or `remove()` on the same set without
// invalidating the in-flight iteration. A listener registered during the
// notification will NOT see the current event (it's added after the
// snapshot); a listener removed during notification WILL still fire once
// for the current event.
//
// Pure C++; no MapLibre deps. Move-only because handles are stable for the
// lifetime of a given ListenerSet and copying would invite confusion about
// which copy a handle refers to.
template <class... Args>
class ListenerSet {
public:
    using Listener = std::function<void(Args...)>;
    using Handle = std::uint64_t;

    ListenerSet() = default;
    ListenerSet(const ListenerSet&) = delete;
    ListenerSet& operator=(const ListenerSet&) = delete;
    ListenerSet(ListenerSet&&) noexcept = default;
    ListenerSet& operator=(ListenerSet&&) noexcept = default;

    Handle add(Listener listener) {
        const Handle h = nextHandle++;
        listeners.emplace(h, std::move(listener));
        return h;
    }

    void remove(Handle handle) { listeners.erase(handle); }

    template <class... CallArgs>
    void notify(CallArgs&&... args) const {
        // Snapshot listeners before iterating so a listener may safely
        // mutate this set during the call. See the reentrancy note above.
        const auto snapshot = listeners;
        for (const auto& [_, listener] : snapshot) {
            listener(args...);
        }
    }

    bool empty() const noexcept { return listeners.empty(); }
    std::size_t size() const noexcept { return listeners.size(); }

private:
    std::unordered_map<Handle, Listener> listeners;
    Handle nextHandle = 0;
};

} // namespace mbgl
