#pragma once

#include <cstdint>
#include <span>

// Thin row-major view over a height grid. Centralises the "sample with
// NaN-on-OOB" helper used by the marching-squares cell loop, and gives
// future overzoom / upsample logic a place to hang. Pure C++; no MapLibre
// deps.

namespace mbgl {
namespace algorithm {
namespace contour {

struct HeightTile {
    // Row-major samples: samples[y * width + x] is the height at column x,
    // row y. Caller owns the storage; HeightTile is a non-owning view.
    std::span<const std::int16_t> samples;
    int width = 0;
    int height = 0;
};

/// Sample at integer grid coordinates. Returns NaN for any out-of-bounds
/// coordinate (including the empty-grid case). Matches the convention used
/// inline in the isolines algorithm so callers can swap to it without
/// re-deriving indexing.
double sample(const HeightTile& tile, int x, int y);

} // namespace contour
} // namespace algorithm
} // namespace mbgl
