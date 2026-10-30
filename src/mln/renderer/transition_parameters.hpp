#pragma once

#include <mln/util/chrono.hpp>
#include <mln/util/feature.hpp>
#include <mln/style/transition_options.hpp>

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace mln {

/// A change of the style's global state, across which the values that read it
/// transition from what they showed before.
class GlobalStateChange {
public:
    /// The state before the change, which transitions already running keep reading.
    std::shared_ptr<const GlobalStateMap> priorState;

    /// The keys whose values differ between the prior and the current state.
    std::shared_ptr<const std::set<std::string>> changedKeys;

    /// Whether the global state is all that changed, so only the values that
    /// read a changed key start a new transition.
    bool stateOnly = false;
};

class TransitionParameters {
public:
    TimePoint now{};
    style::TransitionOptions transition{};

    /// Set when the global state changed since the last transition.
    std::optional<GlobalStateChange> globalStateChange{};

    bool isGlobalStateOnly() const noexcept { return globalStateChange && globalStateChange->stateOnly; }
};

} // namespace mln
