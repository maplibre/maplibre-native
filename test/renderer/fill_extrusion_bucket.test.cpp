#include <mbgl/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mbgl/test/util.hpp>
#include <mbgl/test/stub_geometry_tile_feature.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_properties.hpp>

using namespace mbgl;
using namespace mbgl::style;

namespace {

// A 100x100 tile-unit square ring, closed.
GeometryCollection squareRing() {
    return {{{{0, 0}, {100, 0}, {100, 100}, {0, 100}, {0, 0}}}};
}

// A regular octagon (corner-cut square, cut = 25 tile units on a 100-unit
// square): every one of the 8 corners has an exterior turn of exactly 45
// degrees — gentler than the 60-degree smooth-normal crease threshold
// (kCreaseCos = 0.5 in fill_extrusion_bucket.cpp), unlike the square's 90-
// degree corners. The smooth-normal blending this threshold gates only
// exists on the non-instanced fill-extrusion path (`#if
// !MLN_USE_FILL_EXTRUSION_INSTANCING` in fill_extrusion_bucket.cpp); on
// builds with instancing enabled (e.g. this Metal build,
// MLN_RENDER_BACKEND_METAL=1) that branch is compiled out entirely, so this
// test only verifies radius-0 byte-identity of the shared (instancing-
// agnostic) geometry path. The crease gate itself is exercised on
// non-instanced builds (e.g. Linux GL CI) — before the edge-radius-gate fix,
// an octagon at radius 0 would still get blended (non-faceted) wall normals
// for these gentle turns there.
GeometryCollection octagonRing() {
    return {{{{25, 0}, {75, 0}, {100, 25}, {100, 75}, {75, 100}, {25, 100}, {0, 75}, {0, 25}, {25, 0}}}};
}

FillExtrusionLayoutProperties::PossiblyEvaluated layoutWithRadius(float radius) {
    FillExtrusionLayoutProperties::PossiblyEvaluated layout;
    layout.get<FillExtrusionEdgeRadius>() = radius;
    return layout;
}

std::unique_ptr<FillExtrusionBucket> makeBucket(float radius) {
    auto bucket = std::make_unique<FillExtrusionBucket>(
        layoutWithRadius(radius), std::map<std::string, Immutable<style::LayerProperties>>{}, 15.0f, 0);
    return bucket;
}

void addRing(FillExtrusionBucket& bucket, const GeometryCollection& ring) {
    bucket.addFeature(StubGeometryTileFeature{FeatureType::Polygon, ring.clone()},
                      ring,
                      {},
                      PatternLayerMap(),
                      0,
                      CanonicalTileID(15, 0, 0));
}

// Compares vertex payloads field-by-field. FillExtrusionLayoutVertex has no
// public operator== outside the GL-only test/gl/bucket.test.cpp translation
// unit (which is compiled out entirely under the Metal/Vulkan
// MLN_USE_FILL_EXTRUSION_INSTANCING path this build uses), so a small local
// comparator is cheaper than reintroducing a namespace-wide operator==.
bool vertexEquals(const FillExtrusionLayoutVertex& a, const FillExtrusionLayoutVertex& b) {
    return a.a1 == b.a1 && a.a2 == b.a2;
}

::testing::AssertionResult VerticesMatch(const char* actualExpr,
                                         const char* expectedExpr,
                                         const std::vector<FillExtrusionLayoutVertex>& actual,
                                         const std::vector<FillExtrusionLayoutVertex>& expected) {
    if (actual.size() != expected.size()) {
        return ::testing::AssertionFailure() << actualExpr << ".size() (" << actual.size() << ") != " << expectedExpr
                                             << ".size() (" << expected.size() << ")";
    }
    for (std::size_t i = 0; i < actual.size(); ++i) {
        if (!vertexEquals(actual[i], expected[i])) {
            return ::testing::AssertionFailure()
                   << actualExpr << "[" << i << "] pos=(" << actual[i].a1[0] << "," << actual[i].a1[1]
                   << ") ed_discard=(" << actual[i].a2[0] << "," << actual[i].a2[1] << ") != " << expectedExpr << "["
                   << i << "] pos=(" << expected[i].a1[0] << "," << expected[i].a1[1] << ") ed_discard=("
                   << expected[i].a2[0] << "," << expected[i].a2[1] << ")";
        }
    }
    return ::testing::AssertionSuccess();
}

} // namespace

// Golden geometry captured from stock (pre-fork) upstream code, per the
// task's Step 2b procedure: a throwaway harness ran the identical fixture
// against `FillExtrusionBucket` constructed with `style::Properties<>::
// PossiblyEvaluated{}` (the stock ctor, no edge-radius concept at all) in the
// `pr0-is3d` worktree (pure upstream fill-extrusion code), printing every
// vertex/index. Those printed values are the constants below. This build
// compiles with MLN_RENDER_BACKEND_METAL=1, so
// MLN_USE_FILL_EXTRUSION_INSTANCING is 1 and FillExtrusionLayoutVertex's
// second attribute is `ed_discard` (edgeDistance, isDiscarded) — confirmed
// via sizeof(FillExtrusionLayoutVertex) == 8 during golden capture.
TEST(FillExtrusionBucket, EdgeRadiusZeroSquareMatchesStockGeometry) {
    auto bucket = makeBucket(0.0f);
    addRing(*bucket, squareRing());

    const std::vector<FillExtrusionLayoutVertex> golden = {
        FillExtrusionBucket::layoutVertex({0, 0}, 0, false),
        FillExtrusionBucket::layoutVertex({100, 0}, 100, false),
        FillExtrusionBucket::layoutVertex({100, 100}, 200, false),
        FillExtrusionBucket::layoutVertex({0, 100}, 300, false),
        FillExtrusionBucket::layoutVertex({0, 0}, 400, true),
    };
    EXPECT_PRED_FORMAT2(VerticesMatch, bucket->vertices.vector(), golden);

    const std::vector<uint16_t> goldenIndices = {3, 1, 0, 1, 3, 2};
    EXPECT_EQ(goldenIndices, bucket->triangles.vector());
}

TEST(FillExtrusionBucket, EdgeRadiusZeroOctagonMatchesStockGeometry) {
    auto bucket = makeBucket(0.0f);
    addRing(*bucket, octagonRing());

    const std::vector<FillExtrusionLayoutVertex> golden = {
        FillExtrusionBucket::layoutVertex({25, 0}, 0, false),
        FillExtrusionBucket::layoutVertex({75, 0}, 50, false),
        FillExtrusionBucket::layoutVertex({100, 25}, 85, false),
        FillExtrusionBucket::layoutVertex({100, 75}, 135, false),
        FillExtrusionBucket::layoutVertex({75, 100}, 170, false),
        FillExtrusionBucket::layoutVertex({25, 100}, 220, false),
        FillExtrusionBucket::layoutVertex({0, 75}, 255, false),
        FillExtrusionBucket::layoutVertex({0, 25}, 305, false),
        FillExtrusionBucket::layoutVertex({25, 0}, 340, true),
    };
    EXPECT_PRED_FORMAT2(VerticesMatch, bucket->vertices.vector(), golden);

    const std::vector<uint16_t> goldenIndices = {7, 1, 0, 1, 3, 2, 3, 5, 4, 5, 7, 6, 7, 3, 1, 3, 7, 5};
    EXPECT_EQ(goldenIndices, bucket->triangles.vector());
}

// radius > 0 rounds each footprint corner into a short arc (roundPolygonCorners
// in fill_extrusion_bucket.cpp), replacing each single corner point with
// several arc points — strictly more ring points, hence strictly more
// emitted vertices, than the unrounded radius-0 geometry above. This isn't
// gated by MLN_USE_FILL_EXTRUSION_INSTANCING (unlike the smooth-normal
// blending), so it holds under this Metal build too.
TEST(FillExtrusionBucket, EdgeRadiusPositiveAddsVertices) {
    auto bucketZero = makeBucket(0.0f);
    addRing(*bucketZero, squareRing());

    auto bucketRounded = makeBucket(2.0f);
    addRing(*bucketRounded, squareRing());

    EXPECT_GT(bucketRounded->vertices.elements(), bucketZero->vertices.elements());
}

namespace {
// A long, thin (100 x 4 tile-unit) rectangle: the two short edges force the
// corner-cut at each end to clamp to ~1.5 units (inLen*0.5 - 0.5), so the four
// arc samples (start, two bezier mids, end) fall within ~1-2 units of each
// other. Rounded to int16 several of them collapse onto the SAME integer
// coordinate. This is the exact degeneracy from Sergey's report — new arc
// points quantized to short lose sub-unit precision — reproduced minimally.
GeometryCollection thinRectangleRing() {
    return {{{{0, 0}, {100, 0}, {100, 4}, {0, 4}, {0, 0}}}};
}

// True if any two CONSECUTIVE emitted wall vertices share the same footprint
// position (a1). On the instanced path each ring vertex is emitted once in
// order, so a consecutive position-duplicate is a zero-length wall edge — its
// perpendicular (hence directional-light normal) is undefined → (0,0,0), the
// dark facet/sliver bug. The ring's closing vertex repeats the first point but
// is NOT adjacent to it in the array, so it is not a false positive.
bool hasConsecutivePositionDuplicate(const FillExtrusionBucket& bucket) {
    const auto& v = bucket.vertices.vector();
    for (std::size_t i = 1; i < v.size(); ++i) {
        if (v[i].a1 == v[i - 1].a1) {
            return true;
        }
    }
    return false;
}
} // namespace

// Red/green guard for the edge-radius quantization-degeneracy fix. Rounding a
// corner emits arc points computed in double but stored as int16; when samples
// quantize onto the same texel the wall edge between them collapses to zero
// length, and its shader-computed perpendicular degenerates to (0,0,0) — a
// fully-unlit facet next to correctly-lit neighbours (the half-dark wall split;
// the NYC corner slivers/fin). Before the fix the thin rectangle produces such
// consecutive duplicate positions; the dedup at emit removes every zero-length
// (and folded back-spike) segment, so no wall edge is degenerate.
TEST(FillExtrusionBucket, EdgeRadiusThinRectangleNoDegenerateWallEdges) {
    auto bucket = makeBucket(2.0f);
    addRing(*bucket, thinRectangleRing());

    // The fixture must actually round (else the guard is vacuous): rounding adds
    // arc vertices beyond the 5 of the unrounded ring.
    auto bucketZero = makeBucket(0.0f);
    addRing(*bucketZero, thinRectangleRing());
    ASSERT_GT(bucket->vertices.elements(), bucketZero->vertices.elements())
        << "fixture did not round — cannot exercise the degeneracy";

    EXPECT_FALSE(hasConsecutivePositionDuplicate(*bucket))
        << "a rounded corner emitted a zero-length wall edge (quantization degeneracy)";
}

// The dedup must NOT touch the radius-0 path: rounding never runs there, so the
// geometry stays byte-identical to stock upstream (guarded in full by the
// EdgeRadiusZero* golden tests; this pins the thin-rectangle fixture too).
TEST(FillExtrusionBucket, EdgeRadiusZeroThinRectangleUnchanged) {
    auto bucket = makeBucket(0.0f);
    addRing(*bucket, thinRectangleRing());

    const std::vector<FillExtrusionLayoutVertex> golden = {
        FillExtrusionBucket::layoutVertex({0, 0}, 0, false),
        FillExtrusionBucket::layoutVertex({100, 0}, 100, false),
        FillExtrusionBucket::layoutVertex({100, 4}, 104, false),
        FillExtrusionBucket::layoutVertex({0, 4}, 204, false),
        FillExtrusionBucket::layoutVertex({0, 0}, 208, true),
    };
    EXPECT_PRED_FORMAT2(VerticesMatch, bucket->vertices.vector(), golden);
}

namespace {
// Reconstruct the plan-view footprint ring the bucket actually emitted. On the
// instanced fill-extrusion path (this Metal/Vulkan build,
// MLN_USE_FILL_EXTRUSION_INSTANCING == 1) each footprint outline vertex is
// emitted exactly once, in ring order, with its tile-unit position in a1 — so
// the vertex position stream IS the rounded ring. (On the non-instanced GL path
// the same ring is walked but expanded into wall quads; these ring-shape
// invariants are checked here on the instanced build the test suite compiles.)
GeometryCoordinates emittedFootprint(const FillExtrusionBucket& bucket) {
    GeometryCoordinates ring;
    for (const auto& v : bucket.vertices.vector()) {
        ring.emplace_back(v.a1[0], v.a1[1]);
    }
    return ring;
}

// Twice the signed area of a ring (open form); only the sign is used, as a
// winding-order / orientation proxy.
double footprintDoubleSignedArea(const GeometryCoordinates& ring) {
    std::size_t n = ring.size();
    if (n >= 2 && ring.front() == ring.back()) n -= 1;
    double sum = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const auto& p = ring[i];
        const auto& q = ring[(i + 1) % n];
        sum += static_cast<double>(p.x) * q.y - static_cast<double>(q.x) * p.y;
    }
    return sum;
}

// True if any two non-adjacent edges of the ring cross properly (strict interior
// crossing). A self-intersecting footprint breaks earcut (missing/degenerate
// roof cap) and winds wall quads inside-out — the hollow "U-trough" buildings.
bool footprintSelfIntersects(const GeometryCoordinates& ring) {
    std::size_t n = ring.size();
    if (n >= 2 && ring.front() == ring.back()) n -= 1;
    if (n < 4) return false;
    const auto orient = [](const GeometryCoordinate& o, const GeometryCoordinate& a, const GeometryCoordinate& b) {
        return (static_cast<double>(a.x) - o.x) * (static_cast<double>(b.y) - o.y) -
               (static_cast<double>(a.y) - o.y) * (static_cast<double>(b.x) - o.x);
    };
    const auto properCross = [&](const GeometryCoordinate& p1,
                                 const GeometryCoordinate& p2,
                                 const GeometryCoordinate& p3,
                                 const GeometryCoordinate& p4) {
        const double d1 = orient(p3, p4, p1), d2 = orient(p3, p4, p2);
        const double d3 = orient(p1, p2, p3), d4 = orient(p1, p2, p4);
        return ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) && ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0));
    };
    for (std::size_t i = 0; i < n; ++i) {
        const auto& a = ring[i];
        const auto& b = ring[(i + 1) % n];
        for (std::size_t j = i + 2; j < n; ++j) {
            if (i == 0 && j == n - 1) continue; // wrap-adjacent edges share a vertex
            if (properCross(a, b, ring[j], ring[(j + 1) % n])) return true;
        }
    }
    return false;
}

// A small, simple (non-self-intersecting) footprint with sharp corners and one
// short edge, at a scale where — at z15 with a 2 m radius, radiusUnits ~= 13.4 —
// the quadratic-bezier arc samples of the sharp corners quantize into a local
// zigzag and the arcs of two nearby corners cross. This is a minimal, clean
// stand-in for the small/overzoomed real building footprints that turned up
// hollow and roofless on device with the edge-radius feature on. Found by a
// fuzzer over simple rings; verified simple and CCW below.
GeometryCollection sharpCornerFootprint() {
    return {{{{247, 178}, {265, 230}, {168, 270}, {118, 223}, {138, 191}, {175, 123}, {248, 181}, {247, 178}}}};
}
} // namespace

// RED/GREEN guard for the edge-radius ring-reversal / self-intersection bug (THE
// release blocker). The per-corner dedup folds only an immediate A->B->A back-
// spike; it cannot see a wider tangle. On this footprint the sharp corners round
// into arcs that quantize and cross, so BEFORE the fix the emitted footprint
// self-intersects — which makes earcut drop/degenerate the roof cap and winds
// the wall quads inside-out (culled to a hollow shell). The orientation +
// self-intersection backstop in roundRingCorners catches this and degrades the
// ring to its original faceted corners (which always render solid), so AFTER the
// fix the emitted footprint is simple, keeps its winding, and still caps a roof.
TEST(FillExtrusionBucket, EdgeRadiusSharpFootprintStaysSimpleAndCapped) {
    // Sanity: the input footprint really is a simple ring to begin with.
    auto zero = makeBucket(0.0f);
    addRing(*zero, sharpCornerFootprint());
    const GeometryCoordinates unrounded = emittedFootprint(*zero);
    ASSERT_FALSE(footprintSelfIntersects(unrounded)) << "fixture is not a simple ring";

    auto rounded = makeBucket(2.0f);
    addRing(*rounded, sharpCornerFootprint());
    const GeometryCoordinates footprint = emittedFootprint(*rounded);

    // The emitted footprint must never self-cross (the ring-reversal bug).
    EXPECT_FALSE(footprintSelfIntersects(footprint)) << "rounded footprint self-intersects — hollow/roofless building";

    // Orientation (winding) must be preserved relative to the unrounded ring.
    EXPECT_EQ(footprintDoubleSignedArea(footprint) > 0.0, footprintDoubleSignedArea(unrounded) > 0.0)
        << "rounded footprint reversed winding";

    // Earcut must still produce a roof cap (indices are the roof triangles on the
    // instanced path); a self-intersecting ring would starve it.
    EXPECT_GT(rounded->triangles.elements(), 0u) << "no roof triangles emitted";
    EXPECT_EQ(rounded->triangles.elements() % 3u, 0u);
}

// The backstop must not disturb footprints that round safely: a plain square
// still rounds (more vertices than radius 0) and keeps its winding + simplicity.
TEST(FillExtrusionBucket, EdgeRadiusSafeCornerStillRoundsPreservingWinding) {
    auto zero = makeBucket(0.0f);
    addRing(*zero, squareRing());
    const GeometryCoordinates unrounded = emittedFootprint(*zero);

    auto rounded = makeBucket(2.0f);
    addRing(*rounded, squareRing());
    const GeometryCoordinates footprint = emittedFootprint(*rounded);

    EXPECT_GT(footprint.size(), unrounded.size()) << "square did not round (backstop over-triggered)";
    EXPECT_FALSE(footprintSelfIntersects(footprint));
    EXPECT_EQ(footprintDoubleSignedArea(footprint) > 0.0, footprintDoubleSignedArea(unrounded) > 0.0);
}
