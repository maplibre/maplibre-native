#pragma once

#include <mbgl/util/chrono.hpp>

#include <mapbox/compatibility/value.hpp>

#include <mbgl/util/unitbezier.hpp>
#include <optional>

namespace mbgl {
namespace style {

class TransitionOptions {
public:
    std::optional<Duration> duration;
    std::optional<Duration> delay;
    std::optional<util::UnitBezier> ease;
    bool enablePlacementTransitions;

    TransitionOptions(std::optional<Duration> duration_ = std::nullopt,
                      std::optional<Duration> delay_ = std::nullopt,
                      std::optional<util::UnitBezier> ease_ = std::nullopt,
                      bool enablePlacementTransitions_ = true)
        : duration(duration_ ? std::move(duration_) : std::nullopt),
          delay(delay_ ? std::move(delay_) : std::nullopt),
          ease(ease_ ? std::move(ease_) : std::nullopt),
          enablePlacementTransitions(enablePlacementTransitions_) {}

    TransitionOptions reverseMerge(const TransitionOptions& defaults) const {
        return {duration ? duration : defaults.duration,
                delay ? delay : defaults.delay,
                ease ? ease : defaults.ease,
                enablePlacementTransitions};
    }

    bool isDefined() const { return duration || delay; }

    mapbox::base::Value serialize() const {
        mapbox::base::ValueObject result;
        if (duration) {
            result.emplace("duration", std::chrono::duration_cast<std::chrono::milliseconds>(*duration).count());
        }
        if (delay) {
            result.emplace("delay", std::chrono::duration_cast<std::chrono::milliseconds>(*delay).count());
        }
        return result;
    }
};

} // namespace style
} // namespace mbgl
