#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

// Marching-squares contour-line generation. Pure algorithm, no MapLibre
// dependencies — testable in isolation, reusable outside the renderer.
//
// - Single-pass cell traversal: emits all threshold levels per cell in one
//   sweep through the grid (~5× faster than per-level passes for typical
//   contour-density tiles).
// - Incremental polyline stitching: maintains start/end fragment maps during
//   traversal, no post-pass.
// - Saddle cells (cases 5, 10) emit both segments of one fixed pairing —
//   sufficient for typical DEM data where saddle points are sparse and
//   visually subtle. Center-average disambiguation is a future improvement.
// - Output in tile-local integer coordinates (default extent 4096), suitable
//   for direct emission into vector-tile features.

namespace mbgl {
namespace algorithm {
namespace contour {

struct ContourThresholds {
    // Vertical distance between contours in the same units as the grid.
    double interval = 0.0;
    // Tile-local coordinate scale (MVT default = 4096).
    int extent = 4096;
    // Pixels of skirt to extend output past the tile edge on every side.
    // Useful for clipping at the consumer; matches `maplibre-contour`'s
    // `buffer` parameter.
    int buffer = 1;
};

struct ContourLineString {
    // Threshold value this line traces, in the same units as the input grid.
    double elevation = 0.0;
    // Interleaved tile-local (x, y) integer coordinates: points[0] = x0,
    // points[1] = y0, points[2] = x1, points[3] = y1, … Always even length.
    std::vector<std::int32_t> points;
};

/// Generate contour lines from a height grid.
///
/// @param heights row-major height samples. heights[y*width + x] is the
///   height at the top-left corner of cell (x, y). Grid is treated as a
///   sample lattice of (width × height) values, producing
///   ((width-1) × (height-1)) cells.
/// @param width   number of columns of samples. Must be ≥ 2.
/// @param height  number of rows of samples. Must be ≥ 2.
/// @param thresholds configuration; `interval` must be > 0.
/// @return One ContourLineString per stitched polyline. Lines are not
///   simplified or smoothed — that is the consumer's responsibility.
std::vector<ContourLineString> generateContours(std::span<const std::int16_t> heights,
                                                int width,
                                                int height,
                                                const ContourThresholds& thresholds);

} // namespace contour
} // namespace algorithm
} // namespace mbgl
