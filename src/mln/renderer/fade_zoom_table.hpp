#pragma once

#include <mln/shaders/layer_ubo.hpp>
#include <mln/util/chrono.hpp>

#include <cstdint>
#include <optional>

namespace mln {

/// Opacities for the `line-fade-zoom` and `fill-fade-zoom` paint properties.
///
/// A feature with fade zoom `z` is drawn while the map's zoom is at or above `z`. When the map's zoom crosses `z`,
/// the feature fades in or out over the fade duration, as symbols do when their placement changes. The opacity
/// therefore depends only on the fade zoom and on how the map's zoom moved, not on the feature or its tile: one
/// table, indexed by the fade zoom rounded to `1 / shaders::fadeZoomStepsPerZoom`, serves every line and fill layer.
class FadeZoomTable {
public:
    using Values = std::array<float, shaders::fadeZoomTableSize>;

    FadeZoomTable();

    /// Moves every entry towards its target for the map's `zoom` at `now`. With a zero `duration`, or on the first
    /// update, entries take their target at once. A fade starts in the update that sees the zoom cross an entry and
    /// advances with the time between later updates. Returns true while an entry is still fading.
    bool update(double zoom, TimePoint now, Duration duration);

    /// Index of the entry used for features with fade zoom `fadeZoom`, as the shaders compute it.
    static std::size_t index(float fadeZoom);

    /// Current opacity for features with fade zoom `fadeZoom`.
    float opacity(float fadeZoom) const { return values[index(fadeZoom)]; }

    const Values& getValues() const { return values; }

    /// Incremented whenever an entry changes, so that the table is uploaded only when needed.
    uint64_t getVersion() const { return version; }

    bool isFading() const { return fading; }

private:
    Values values;
    std::optional<TimePoint> lastUpdate;
    uint64_t version = 0;
    bool fading = false;
};

} // namespace mln
