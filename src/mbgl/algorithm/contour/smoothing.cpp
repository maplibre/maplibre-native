#include <mbgl/algorithm/contour/smoothing.hpp>

#include <cmath>
#include <cstddef>
#include <utility>

namespace mbgl {
namespace algorithm {
namespace contour {

namespace {

// Perpendicular distance from p to the line through a, b. Falls back to
// point-to-point distance when a == b.
double perpDist(const Point2D& p, const Point2D& a, const Point2D& b) {
    const double dx = b[0] - a[0];
    const double dy = b[1] - a[1];
    const double lenSq = dx * dx + dy * dy;
    const double px = p[0] - a[0];
    const double py = p[1] - a[1];
    if (lenSq == 0.0) {
        return std::sqrt(px * px + py * py);
    }
    const double cross = std::abs(dx * py - dy * px);
    return cross / std::sqrt(lenSq);
}

// Recursive RDP. Returns indices of retained points in `keep` (sorted, with
// the first and last index always present in the caller). Recursion depth
// is bounded by the polyline length, which for DEM-derived contour tiles
// is small enough not to need an explicit work-stack.
void rdp(std::span<const Point2D> pts, double epsilon, std::size_t lo, std::size_t hi, std::vector<std::size_t>& keep) {
    if (hi <= lo + 1) return;
    double dmax = 0.0;
    std::size_t index = lo;
    for (std::size_t i = lo + 1; i < hi; i++) {
        const double d = perpDist(pts[i], pts[lo], pts[hi]);
        if (d > dmax) {
            dmax = d;
            index = i;
        }
    }
    if (dmax > epsilon) {
        rdp(pts, epsilon, lo, index, keep);
        keep.push_back(index);
        rdp(pts, epsilon, index, hi, keep);
    }
}

} // namespace

std::vector<Point2D> douglasPeucker(std::span<const Point2D> points, double epsilon) {
    std::vector<Point2D> out;
    if (points.size() < 2) {
        out.assign(points.begin(), points.end());
        return out;
    }
    if (points.size() == 2) {
        out.assign(points.begin(), points.end());
        return out;
    }
    std::vector<std::size_t> keep;
    keep.reserve(points.size());
    keep.push_back(0);
    rdp(points, epsilon, 0, points.size() - 1, keep);
    keep.push_back(points.size() - 1);
    out.reserve(keep.size());
    for (auto i : keep) out.push_back(points[i]);
    return out;
}

std::vector<Point2D> chaikin(std::span<const Point2D> points, int iterations) {
    std::vector<Point2D> out(points.begin(), points.end());
    if (iterations <= 0 || out.size() < 3) return out;

    for (int iter = 0; iter < iterations; iter++) {
        const std::size_t n = out.size();
        if (n < 3) break;
        std::vector<Point2D> next;
        next.reserve(2 * n - 2);
        next.push_back(out[0]); // pinned start

        for (std::size_t i = 0; i + 1 < n; i++) {
            const Point2D& p0 = out[i];
            const Point2D& p1 = out[i + 1];
            const Point2D q{0.75 * p0[0] + 0.25 * p1[0], 0.75 * p0[1] + 0.25 * p1[1]};
            const Point2D s{0.25 * p0[0] + 0.75 * p1[0], 0.25 * p0[1] + 0.75 * p1[1]};
            if (i == 0) {
                // Skip q — sits next to the pinned start.
                next.push_back(s);
            } else if (i == n - 2) {
                // Skip s — sits next to the pinned end.
                next.push_back(q);
            } else {
                next.push_back(q);
                next.push_back(s);
            }
        }
        next.push_back(out[n - 1]); // pinned end
        out = std::move(next);
    }
    return out;
}

} // namespace contour
} // namespace algorithm
} // namespace mbgl
