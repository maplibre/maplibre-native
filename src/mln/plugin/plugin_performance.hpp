#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>

namespace mln::plugin::performance {

// Internal opt-in diagnostics: disabled scopes do not read the clock or mutate counters.
enum Stage : std::size_t {
    Layout,
    Snapshots,
    Binding,
    State,
    Count
};
inline std::atomic<bool> enabled{false};
inline std::array<std::atomic<uint64_t>, Count> nanoseconds{};
inline std::atomic<uint64_t> snapshots{0};
inline std::atomic<uint64_t> stateRanges{0};
struct Snapshot {
    std::array<uint64_t, Count> nanoseconds{};
    uint64_t snapshots = 0;
    uint64_t stateRanges = 0;
};
inline Snapshot read() {
    Snapshot result;
    for (std::size_t i = 0; i < Count; ++i) result.nanoseconds[i] = nanoseconds[i].load(std::memory_order_relaxed);
    result.snapshots = snapshots.load(std::memory_order_relaxed);
    result.stateRanges = stateRanges.load(std::memory_order_relaxed);
    return result;
}
class Scope {
public:
    explicit Scope(Stage stage_)
        : stage(stage_),
          active(enabled.load(std::memory_order_relaxed)) {
        if (active) start = Clock::now();
    }
    ~Scope() { stop(); }
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    void stop() {
        if (!active) return;
        nanoseconds[stage].fetch_add(std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count(),
                                     std::memory_order_relaxed);
        active = false;
    }
    bool isActive() const { return active; }

private:
    using Clock = std::chrono::steady_clock;
    Stage stage;
    bool active;
    Clock::time_point start;
};
} // namespace mln::plugin::performance
