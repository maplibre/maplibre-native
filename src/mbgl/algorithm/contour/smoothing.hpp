#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <vector>

// Smoothing primitives applied to raw marching-squares output before MVT
// emission. Two stages: Ramer-Douglas-Peucker simplification followed by
// Chaikin's corner-cutting. Pure C++; no MapLibre deps.

namespace mbgl {
namespace algorithm {
namespace contour {

// Working point type for the smoothing pipeline. doubles preserve Chaikin's
// fractional interpolation across iterations; the consumer rounds to the
// integer MVT extent at the very end.
using Point2D = std::array<double, 2>;

/// Ramer–Douglas–Peucker polyline simplification. Returns a subset of
/// `points` (preserving order, first, last) where every dropped point sits
/// at perpendicular distance ≤ `epsilon` from the segment between its
/// retained neighbours. Strict-greater convention: a point exactly at the
/// threshold is dropped.
std::vector<Point2D> douglasPeucker(std::span<const Point2D> points, double epsilon);

/// Chaikin's corner-cutting smoothing applied iteratively. Endpoints are
/// pinned exactly; interior segments contribute two new points (the 1/4 and
/// 3/4 interpolants), with each end-segment dropping its inner cut to avoid
/// a near-coincident duplicate of the pinned endpoint. n iterations grow
/// the vertex count to (2n - 2) per iteration.
///
/// `iterations <= 0` or fewer than 3 input points → returns input unchanged.
std::vector<Point2D> chaikin(std::span<const Point2D> points, int iterations);

} // namespace contour
} // namespace algorithm
} // namespace mbgl
