#include <mbgl/algorithm/contour/isolines.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <limits>
#include <unordered_map>

namespace mbgl {
namespace algorithm {
namespace contour {

namespace {

// CASES[idx][i] = i-th segment for marching-squares case `idx`.
// Each segment is { startEdge, endEdge } where each edge is one of:
//   {1, 0} top      — between TL and TR
//   {2, 1} right    — between TR and BR
//   {1, 2} bottom   — between BL and BR
//   {0, 1} left     — between TL and BL
//
// Bit layout of `idx`: TL=8 | TR=4 | BR=2 | BL=1, set if the corresponding
// sample is *strictly above* the threshold. Cases 0 and 15 (all below / all
// above) emit no segments. Cases 5 and 10 are saddle configurations and emit
// two segments — saddle disambiguation is applied later.
struct Edge {
    std::int8_t u; // 0=left  1=mid  2=right (along the cell's x axis)
    std::int8_t v; // 0=top   1=mid  2=bottom (along the cell's y axis)
};
struct Segment {
    Edge start;
    Edge end;
};

constexpr std::array<std::array<Segment, 2>, 16> CASES = {{
    /*  0 ----  */ {Segment{{0, 0}, {0, 0}}, Segment{{0, 0}, {0, 0}}},
    /*  1 ---BL */ {Segment{{1, 2}, {0, 1}}, Segment{{0, 0}, {0, 0}}},
    /*  2 --BR- */ {Segment{{2, 1}, {1, 2}}, Segment{{0, 0}, {0, 0}}},
    /*  3 --BR+BL (bottom row) */ {Segment{{2, 1}, {0, 1}}, Segment{{0, 0}, {0, 0}}},
    /*  4 -TR-- */ {Segment{{1, 0}, {2, 1}}, Segment{{0, 0}, {0, 0}}},
    /*  5 -TR+BL (saddle) */ {Segment{{1, 2}, {0, 1}}, Segment{{1, 0}, {2, 1}}},
    /*  6 -TR+BR (right column) */ {Segment{{1, 0}, {1, 2}}, Segment{{0, 0}, {0, 0}}},
    /*  7 -TR+BR+BL (only TL below) */ {Segment{{1, 0}, {0, 1}}, Segment{{0, 0}, {0, 0}}},
    /*  8 TL--- */ {Segment{{0, 1}, {1, 0}}, Segment{{0, 0}, {0, 0}}},
    /*  9 TL+BL (left column) */ {Segment{{1, 2}, {1, 0}}, Segment{{0, 0}, {0, 0}}},
    /* 10 TL+BR (saddle) */ {Segment{{0, 1}, {1, 0}}, Segment{{2, 1}, {1, 2}}},
    /* 11 TL+BR+BL (only TR below) */ {Segment{{2, 1}, {1, 0}}, Segment{{0, 0}, {0, 0}}},
    /* 12 TL+TR (top row) */ {Segment{{0, 1}, {2, 1}}, Segment{{0, 0}, {0, 0}}},
    /* 13 TL+TR+BL (only BR below) */ {Segment{{1, 2}, {2, 1}}, Segment{{0, 0}, {0, 0}}},
    /* 14 TL+TR+BR (only BL below) */ {Segment{{0, 1}, {1, 2}}, Segment{{0, 0}, {0, 0}}},
    /* 15 ---- */ {Segment{{0, 0}, {0, 0}}, Segment{{0, 0}, {0, 0}}},
}};

constexpr std::array<int, 16> SEGMENT_COUNT = {0, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 0};

inline double ratio(double a, double threshold, double b) {
    return (threshold - a) / (b - a);
}

inline void edgePosition(const Edge edge, int c, int r, double tl, double tr, double bl, double br,
                         double threshold, double& outX, double& outY) {
    if (edge.u == 0) {
        outX = static_cast<double>(c - 1);
        outY = static_cast<double>(r) - ratio(bl, threshold, tl);
    } else if (edge.u == 2) {
        outX = static_cast<double>(c);
        outY = static_cast<double>(r) - ratio(br, threshold, tr);
    } else if (edge.v == 0) {
        outX = static_cast<double>(c) - ratio(tr, threshold, tl);
        outY = static_cast<double>(r - 1);
    } else {
        outX = static_cast<double>(c) - ratio(br, threshold, bl);
        outY = static_cast<double>(r);
    }
}

inline std::int32_t scaleCoord(double v, double multiplier) {
    return static_cast<std::int32_t>(std::lround(v * multiplier));
}

// Edge slots are unique across the whole grid. Each cell occupies a 2×2
// block of slots; adjacent cells share their boundary slots. The encoding
// matches `maplibre-contour`'s isolines.ts so the algorithm produces the
// same connectivity. Returns a 64-bit packed slot index — comfortably within
// range for any practical tile size.
inline std::uint64_t edgeIndex(int width, int c, int r, Edge edge) {
    const std::int64_t x = static_cast<std::int64_t>(c) * 2 + edge.u;
    const std::int64_t y = static_cast<std::int64_t>(r) * 2 + edge.v;
    const std::int64_t stride = static_cast<std::int64_t>(width + 1) * 2;
    return static_cast<std::uint64_t>(x + y * stride);
}

// One in-progress polyline at a single threshold. `start` and `end` are edge
// slot indices; `points` holds interleaved (x, y) tile-local int coords.
// `merged` is set when this fragment has been folded into another and should
// be ignored in the final emission pass.
//
// `points` is a deque (not vector) because polylines stitch from both ends:
// each segment may extend the head or the tail of the fragment, and a vector
// would force an O(N) shift on every head extension. Deque's O(1) push_front
// keeps the inner-loop cost per segment constant; the per-emit copy into the
// public std::vector is paid once per polyline, replacing N shifts incurred
// during stitching.
struct Fragment {
    std::uint64_t start;
    std::uint64_t end;
    std::deque<std::int32_t> points;
    bool merged = false;
};

struct LevelState {
    std::vector<Fragment> fragments;
    std::unordered_map<std::uint64_t, std::size_t> byStart;
    std::unordered_map<std::uint64_t, std::size_t> byEnd;
};

inline void appendPoint(Fragment& f, std::int32_t x, std::int32_t y) {
    f.points.push_back(x);
    f.points.push_back(y);
}

inline void prependPoint(Fragment& f, std::int32_t x, std::int32_t y) {
    // push_front pushes one element at a time; to land (x, y) at the front
    // of the interleaved sequence in that order we push y first, then x.
    f.points.push_front(y);
    f.points.push_front(x);
}

} // namespace

std::vector<ContourLineString> generateContours(std::span<const std::int16_t> heights,
                                                int width,
                                                int height,
                                                const ContourThresholds& thresholds) {
    if (thresholds.interval <= 0.0) {
        return {};
    }
    if (width < 2 || height < 2) {
        return {};
    }
    if (static_cast<std::ptrdiff_t>(heights.size()) < static_cast<std::ptrdiff_t>(width) * height) {
        return {};
    }

    const double interval = thresholds.interval;
    const double multiplier = static_cast<double>(thresholds.extent) / static_cast<double>(width - 1);

    // Per-threshold polyline accumulators. We key by level rounded to the
    // nearest interval-step, expressed as a signed integer multiple, to avoid
    // floating-point map-key headaches.
    std::unordered_map<long long, LevelState> levels;

    auto sample = [&](int x, int y) -> double {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return static_cast<double>(heights[y * width + x]);
    };

    // Iterate cells in [1, width) × [1, height): each cell at (c, r)
    // samples corners at (c-1, r-1), (c, r-1), (c-1, r), (c, r), so this
    // covers all (width-1) × (height-1) cells exactly once. Skirts beyond
    // the grid edge are the caller's responsibility (e.g. by passing a
    // border-padded height grid).
    for (int r = 1; r < height; r++) {
        for (int c = 1; c < width; c++) {
            const double tl = sample(c - 1, r - 1);
            const double tr = sample(c, r - 1);
            const double bl = sample(c - 1, r);
            const double br = sample(c, r);
            if (std::isnan(tl) || std::isnan(tr) || std::isnan(bl) || std::isnan(br)) {
                continue;
            }

            const double minV = std::min({tl, tr, bl, br});
            const double maxV = std::max({tl, tr, bl, br});
            const double startLevel = std::ceil(minV / interval) * interval;
            const double endLevel = std::floor(maxV / interval) * interval;

            for (double level = startLevel; level <= endLevel + 0.5 * interval; level += interval) {
                const int idx = (tl > level ? 8 : 0) | (tr > level ? 4 : 0) | (br > level ? 2 : 0) | (bl > level ? 1 : 0);
                const int nSegments = SEGMENT_COUNT[idx];
                if (nSegments == 0) continue;

                const long long levelKey = static_cast<long long>(std::llround(level / interval));
                LevelState& state = levels[levelKey];

                for (int s = 0; s < nSegments; s++) {
                    const Segment seg = CASES[idx][s];
                    double sx, sy, ex, ey;
                    edgePosition(seg.start, c, r, tl, tr, bl, br, level, sx, sy);
                    edgePosition(seg.end, c, r, tl, tr, bl, br, level, ex, ey);
                    const std::int32_t sxi = scaleCoord(sx, multiplier);
                    const std::int32_t syi = scaleCoord(sy, multiplier);
                    const std::int32_t exi = scaleCoord(ex, multiplier);
                    const std::int32_t eyi = scaleCoord(ey, multiplier);

                    const std::uint64_t startIdx = edgeIndex(width, c, r, seg.start);
                    const std::uint64_t endIdx = edgeIndex(width, c, r, seg.end);

                    auto fByEnd = state.byEnd.find(startIdx);
                    auto fByStart = state.byStart.find(endIdx);

                    if (fByEnd != state.byEnd.end()) {
                        // Some fragment ends at our start. Extend its tail.
                        const std::size_t fIdx = fByEnd->second;
                        Fragment& f = state.fragments[fIdx];
                        state.byEnd.erase(fByEnd);

                        if (fByStart != state.byStart.end()) {
                            const std::size_t gIdx = fByStart->second;
                            state.byStart.erase(fByStart);
                            if (fIdx == gIdx) {
                                // Closing a ring: append the closing point and emit f as-is.
                                appendPoint(f, exi, eyi);
                                // Ring is complete; remove from any maps (already done) and
                                // leave it in `fragments` to be emitted at the end.
                            } else {
                                // Connect two distinct fragments.
                                Fragment& g = state.fragments[gIdx];
                                f.points.insert(f.points.end(), g.points.begin(), g.points.end());
                                f.end = g.end;
                                g.merged = true;
                                state.byEnd[f.end] = fIdx;
                            }
                        } else {
                            // Just extend f's tail with the new endpoint.
                            appendPoint(f, exi, eyi);
                            f.end = endIdx;
                            state.byEnd[endIdx] = fIdx;
                        }
                    } else if (fByStart != state.byStart.end()) {
                        // Some fragment starts at our end. Prepend our start.
                        const std::size_t fIdx = fByStart->second;
                        Fragment& f = state.fragments[fIdx];
                        state.byStart.erase(fByStart);
                        prependPoint(f, sxi, syi);
                        f.start = startIdx;
                        state.byStart[startIdx] = fIdx;
                    } else {
                        // New fragment.
                        const std::size_t fIdx = state.fragments.size();
                        Fragment newF{startIdx, endIdx, std::deque<std::int32_t>{sxi, syi, exi, eyi}, false};
                        state.fragments.push_back(std::move(newF));
                        state.byStart[startIdx] = fIdx;
                        state.byEnd[endIdx] = fIdx;
                    }
                }
            }
        }
    }

    // Collect surviving fragments into ContourLineStrings. The internal
    // deque is copied into the public vector once per polyline — cheaper
    // than the O(N) per-prepend shifting the vector form would have cost.
    std::vector<ContourLineString> result;
    for (auto& [levelKey, state] : levels) {
        const double level = static_cast<double>(levelKey) * interval;
        for (auto& f : state.fragments) {
            if (f.merged) continue;
            if (f.points.size() < 4) continue;
            ContourLineString line;
            line.elevation = level;
            line.points.assign(f.points.begin(), f.points.end());
            result.push_back(std::move(line));
        }
    }

    return result;
}

} // namespace contour
} // namespace algorithm
} // namespace mbgl
