#include <mln/renderer/fade_zoom_table.hpp>

#include <algorithm>
#include <cmath>

namespace mln {

namespace {
// A camera brought to a whole zoom, for example by a double tap, may land a hair below it.
constexpr double zoomEpsilon = 1e-5;
} // namespace

FadeZoomTable::FadeZoomTable() {
    values.fill(0.0f);
}

std::size_t FadeZoomTable::index(float fadeZoom) {
    // Half steps round up, as `floor(x + 0.5)` does in every shader.
    const auto steps = static_cast<long>(
        std::floor(fadeZoom * static_cast<float>(shaders::fadeZoomStepsPerZoom) + 0.5f));
    return static_cast<std::size_t>(std::clamp<long>(steps, 0, static_cast<long>(shaders::fadeZoomTableSize) - 1));
}

bool FadeZoomTable::update(double zoom, TimePoint now, Duration duration) {
    // Time passes for the table only while it fades. A map that stops rendering when nothing moves would
    // otherwise count its idle time, and a fade started after a pause would end in its first frame.
    const bool snap = !lastUpdate || duration <= Duration::zero();
    const float step = snap ? 1.0f
                       : fading
                           ? std::chrono::duration<float>(now - *lastUpdate) / std::chrono::duration<float>(duration)
                           : 0.0f;
    lastUpdate = now;

    bool changed = false;
    fading = false;
    for (std::size_t i = 0; i < values.size(); ++i) {
        const double threshold = static_cast<double>(i) / shaders::fadeZoomStepsPerZoom;
        const float target = (zoom + zoomEpsilon >= threshold) ? 1.0f : 0.0f;
        float& value = values[i];
        if (value == target) {
            continue;
        }
        const float next = (target > value) ? std::min(target, value + step) : std::max(target, value - step);
        changed = changed || next != value;
        value = next;
        fading = fading || value != target;
    }
    if (changed) {
        version++;
    }
    return fading;
}

} // namespace mln
