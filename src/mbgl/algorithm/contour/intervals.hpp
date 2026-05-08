#pragma once

#include <vector>

// Per-zoom contour-interval schedule. Mirrors the contour-source spec's
// `intervals` field, encoded as a step-by-zoom expression: an odd-length
// array where `stops[0]` is the output below the first stop, `stops[1]` is
// the first stop, `stops[2]` is the output between the first and second
// stops, and so on. Step expressions are right-continuous (zoom == stop uses
// the *next* output), matching MapLibre's `step` expression semantics.
//
// Pure C++; no MapLibre deps. Used by the contour-tile builder to choose a
// per-tile contour interval before invoking the marching-squares algorithm.

namespace mbgl {
namespace algorithm {
namespace contour {

struct IntervalSchedule {
    // Always odd length when valid: outputs interleaved with stops.
    // Example: {200, 12, 100, 14, 50, 15, 20} reads as
    //   z<12 → 200, 12≤z<14 → 100, 14≤z<15 → 50, z≥15 → 20.
    std::vector<double> stops;
};

/// Look up the contour interval for a given zoom. Returns 0.0 for an empty
/// or invalid schedule. Zoom is treated as a continuous value; step
/// boundaries are right-continuous.
double intervalForZoom(const IntervalSchedule& schedule, double zoom);

/// True when the schedule has odd length, strictly-increasing stops, and
/// strictly-positive outputs.
bool isValid(const IntervalSchedule& schedule);

} // namespace contour
} // namespace algorithm
} // namespace mbgl
