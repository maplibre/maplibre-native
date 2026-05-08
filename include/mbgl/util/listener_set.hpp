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

    void remove(Handle handle) {
        listeners.erase(handle);
    }

    template <class... CallArgs>
    void notify(CallArgs&&... args) const {
        for (const auto& [_, listener] : listeners) {
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
