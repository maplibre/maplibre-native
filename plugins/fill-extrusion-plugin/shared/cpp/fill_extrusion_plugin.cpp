#include "fill_extrusion_plugin.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <vector>

#ifndef MLN_FILL_EXTRUSION_PLUGIN_VERSION
#define MLN_FILL_EXTRUSION_PLUGIN_VERSION "0.1.0-local"
#endif

// "Option 2" proof-of-concept: a standalone building-extrusion layer plus its own ground shadow,
// both from one shared triangulated mesh per feature. Registered as "fill-extrusion-plugin" (not
// "fill-extrusion" -- a plugin cannot register under a name a built-in layer type already owns).
//
// Option 2 additions over the option-1 pass: the building shader consumes the map's light
// configuration (mln_plugin_uniform_context_v1's light_color/light_position/light_intensity
// fields) for basic per-wall ambient+directional shading plus a vertical-gradient darkening
// toward the base. Both shaders use is3D + shared stencil overlap dedup
// (enable_stencil_overlap_dedup) -- building was previously tried with enable_depth_write (real
// depth test+write, see the doc comment on mln_plugin_shader_descriptor_v1's enable_depth_write
// field) for correct occlusion against other buildings, but that genuinely z-fights whenever two
// different features' surfaces land at nearly the same depth (confirmed via
// plugins/fill-extrusion-plugin/render-tests/core-parity's fill-extrusion-height/function case:
// three overlapping boxes of different sizes/heights render with spiky artifacts at
// enable_depth_write=1, cleanly at 0). This is a real floating-point depth-precision limitation
// -- real fill-extrusion avoids it via sub-pixel "decimals" vertex precision (see the tile-seam
// limitation below) -- not a simple bug, so stencil dedup is used for both shaders for now. The
// tradeoff, deliberately accepted: no real depth occlusion between separate buildings (relies on
// style layer paint order only), and real depth would be wrong for shadow's flattened ground
// geometry regardless.
//
// Known limitations, deliberately accepted:
//  - Only the outer ring of each feature is used; holes are ignored.
//  - fill-extrusion-plugin-color/-height/-base are per-feature data-driven; opacity,
//    vertical-gradient, and every shadow-* property are camera/zoom-only (no per-feature data).
//  - fill-extrusion-plugin-rounded-corner-distance is a real layout property (evaluated once per
//    bucket at layout time, matching real fill-extrusion's own layout property of the same kind --
//    see the layout_properties doc comment on mln_plugin_layer_type_v1), not per-frame like a paint
//    property, since rounding changes triangulated geometry itself. Because the building and its
//    shadow are triangulated from the exact same rounded ring in the same layoutFeature() call,
//    they stay visually consistent with each other regardless of the configured radius.
//  - Small gaps/seams can still appear right at tile boundaries: vector tiles duplicate geometry
//    near tile edges, and each tile computes its copy's position through its own tile matrix, so
//    a shared boundary vertex's two copies are close but not bit-identical. Real fill-extrusion avoids
//    this by encoding vertex positions with sub-pixel "decimals" precision (FillExtrusionLayoutVertex)
//    so a shared boundary vertex produces bit-identical positions across adjacent tiles; this plugin's
//    plain int16 x/y encoding doesn't, so this is accepted as a v1 limitation rather than adding that
//    encoding now.
//  - The wall/roof shading is a simplified ambient+directional approximation, not a byte-for-byte
//    port of fill-extrusion's own lighting formula.
//  - No pattern (image) fill: the plugin ABI has no texture/sampler support at all yet.
namespace {

// Attribute IDs are local to this one registered layer type; they may be (and are) reused
// across this layer type's two shaders (building, shadow) since attribute validation is
// independent per shader.
constexpr uint32_t positionAttribute = 0;
constexpr uint32_t topAttribute = 1;
constexpr uint32_t buildingColorMinAttribute = 2;
constexpr uint32_t buildingColorMaxAttribute = 3;
constexpr uint32_t heightAttribute = 4;
constexpr uint32_t baseAttribute = 5;
constexpr uint32_t buildingOpacityAttribute = 6;
constexpr uint32_t shadowAzimuthAttribute = 7;
constexpr uint32_t shadowLengthAttribute = 8;
constexpr uint32_t shadowOpacityAttribute = 9;
constexpr uint32_t shadowColorMinAttribute = 10;
constexpr uint32_t shadowColorMaxAttribute = 11;
constexpr uint32_t normalAttribute = 12; // building shader only, real (always-bound) mesh data
constexpr uint32_t verticalGradientAttribute = 13; // building shader only, dead (camera-only)

constexpr uint32_t vertexStream = 0;
constexpr uint64_t buildingDrawableKey = 1;
constexpr uint64_t shadowDrawableKey = 2;

// Globally distinct across both shaders, purely so updateUniformBlock() can switch on `id` alone.
constexpr uint32_t buildingDrawableUniformId = 0;
constexpr uint32_t buildingLayerUniformId = 1;
constexpr uint32_t shadowDrawableUniformId = 2;
constexpr uint32_t shadowLayerUniformId = 3;

// Default corner rounding radius, used when fill-extrusion-plugin-rounded-corner-distance is not
// set in the style. Tile units; roughly a couple of metres at typical building-scale zooms.
constexpr double kDefaultRoundedCornerDistance = 4.0;

constexpr mln_plugin_string str(const char* value, size_t size) {
    return {value, size};
}
template <size_t N>
constexpr mln_plugin_string str(const char (&value)[N]) {
    return str(value, N - 1);
}

struct Vertex {
    int16_t x;
    int16_t y;
    uint16_t top; // 0 = base of a wall (z = height/base "base"), 1 = wall top / roof ("height")
    // Outward 2D wall normal, scaled by kNormalScale (building shader only; the shadow shader
    // never declares a_normal, so this data is simply unused/unbound for that drawable). (0, 0)
    // is the roof-cap sentinel -- the shader treats it as a vertical (0, 0, 1) normal instead.
    int16_t nx;
    int16_t ny;
};
static_assert(sizeof(Vertex) == 10);
constexpr float kNormalScale = 32767.0f;

struct Layout {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<mln_plugin_segment_v1> segments;
    std::vector<mln_plugin_feature_vertex_range_v1> featureRanges;
    std::array<mln_plugin_vertex_stream_v1, 1> streams{};
    // The host requires each drawable's attribute_count to exactly match its shader's total
    // declared attributes minus that shader's own property-bound (host-managed) ones. The building
    // shader has one more real (non-property) attribute than the shadow shader -- a_normal -- so
    // they need separate-length arrays even though they share position/top.
    std::array<mln_plugin_attribute_binding_v1, 3> buildingAttributes{};
    std::array<mln_plugin_attribute_binding_v1, 2> shadowAttributes{};
    std::array<mln_plugin_drawable_descriptor_v1, 2> drawables{};
    // Set once in createLayout() from the evaluated fill-extrusion-plugin-rounded-corner-distance
    // layout property; falls back to the default if the host provides no layout property values.
    double roundedCornerDistance = kDefaultRoundedCornerDistance;
};

void startSegment(Layout& layout) {
    mln_plugin_segment_v1 segment{};
    segment.struct_size = sizeof(segment);
    segment.vertex_offset = static_cast<uint32_t>(layout.vertices.size());
    segment.index_offset = static_cast<uint32_t>(layout.indices.size());
    layout.segments.push_back(segment);
}

void reserveVertices(Layout& layout, size_t count) {
    auto& active = layout.segments.back();
    if (active.vertex_length + count > std::numeric_limits<uint16_t>::max()) {
        startSegment(layout);
    }
}

void pushTriangle(Layout& layout, Vertex a, Vertex b, Vertex c) {
    reserveVertices(layout, 3);
    auto& active = layout.segments.back();
    const auto base = static_cast<uint16_t>(active.vertex_length);
    layout.vertices.push_back(a);
    layout.vertices.push_back(b);
    layout.vertices.push_back(c);
    const uint16_t tri[] = {base, static_cast<uint16_t>(base + 1), static_cast<uint16_t>(base + 2)};
    layout.indices.insert(layout.indices.end(), std::begin(tri), std::end(tri));
    active.vertex_length += 3;
    active.index_length += 3;
}

void pushQuad(Layout& layout, Vertex a, Vertex b, Vertex c, Vertex d) {
    reserveVertices(layout, 4);
    auto& active = layout.segments.back();
    const auto base = static_cast<uint16_t>(active.vertex_length);
    layout.vertices.push_back(a);
    layout.vertices.push_back(b);
    layout.vertices.push_back(c);
    layout.vertices.push_back(d);
    const uint16_t quad[] = {base,
                             static_cast<uint16_t>(base + 1),
                             static_cast<uint16_t>(base + 2),
                             base,
                             static_cast<uint16_t>(base + 2),
                             static_cast<uint16_t>(base + 3)};
    layout.indices.insert(layout.indices.end(), std::begin(quad), std::end(quad));
    active.vertex_length += 4;
    active.index_length += 6;
}

struct Point2D {
    double x, y;
};

Point2D operator-(const Point2D& a, const Point2D& b) {
    return {a.x - b.x, a.y - b.y};
}
Point2D operator+(const Point2D& a, const Point2D& b) {
    return {a.x + b.x, a.y + b.y};
}
Point2D operator*(const Point2D& a, double s) {
    return {a.x * s, a.y * s};
}

double cross(const Point2D& a, const Point2D& b) {
    return a.x * b.y - a.y * b.x;
}
double cross(const Point2D& a, const Point2D& b, const Point2D& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
double dot(const Point2D& a, const Point2D& b) {
    return a.x * b.x + a.y * b.y;
}
double length(const Point2D& v) {
    return std::sqrt(dot(v, v));
}
double distance(const Point2D& a, const Point2D& b) {
    return length(b - a);
}
Point2D normalized(const Point2D& v) {
    const double len = length(v);
    return len > 1e-9 ? Point2D{v.x / len, v.y / len} : Point2D{0, 0};
}
Point2D perp(const Point2D& v) {
    return {-v.y, v.x};
}
double angleBetween(const Point2D& a, const Point2D& b) {
    return std::atan2(cross(a, b), dot(a, b));
}

bool pointInTriangle(const Point2D& p, const Point2D& a, const Point2D& b, const Point2D& c, bool ccw) {
    // A point exactly coincident with one of the triangle's own vertices is never "inside" for
    // ear-clipping purposes. This matters for polygons-with-holes: bridgeHoleIntoRing() closes its
    // slit by duplicating a vertex (the hole's bridge point appears twice, as does the outer
    // ring's), and without this check, a candidate ear whose vertex has such a duplicate elsewhere
    // in the ring would see the >=0 (inclusive) test below satisfied trivially by that duplicate,
    // wrongly rejecting an otherwise-valid ear -- which can cascade into rejecting every ear near
    // the bridge and leaving the whole ring untriangulated.
    if ((p.x == a.x && p.y == a.y) || (p.x == b.x && p.y == b.y) || (p.x == c.x && p.y == c.y)) return false;
    const double c0 = cross(a, b, p);
    const double c1 = cross(b, c, p);
    const double c2 = cross(c, a, p);
    return ccw ? (c0 >= 0 && c1 >= 0 && c2 >= 0) : (c0 <= 0 && c1 <= 0 && c2 <= 0);
}

// Simple O(n^2)-ish ear clipping for a single, non-self-intersecting ring without holes.
std::vector<std::array<uint32_t, 3>> triangulateRing(const std::vector<Point2D>& ring) {
    std::vector<std::array<uint32_t, 3>> triangles;
    const size_t n = ring.size();
    if (n < 3) return triangles;

    double signedArea = 0;
    for (size_t i = 0; i < n; ++i) {
        const auto& a = ring[i];
        const auto& b = ring[(i + 1) % n];
        signedArea += (a.x * b.y - b.x * a.y);
    }
    const bool ccw = signedArea > 0;

    std::vector<uint32_t> remaining(n);
    for (uint32_t i = 0; i < n; ++i) remaining[i] = i;

    size_t guard = n * n + 8;
    while (remaining.size() > 3 && guard-- > 0) {
        bool clipped = false;
        const size_t m = remaining.size();
        for (size_t i = 0; i < m; ++i) {
            const size_t iPrev = (i + m - 1) % m;
            const size_t iNext = (i + 1) % m;
            const auto& a = ring[remaining[iPrev]];
            const auto& b = ring[remaining[i]];
            const auto& c = ring[remaining[iNext]];
            const double turn = cross(a, b, c);
            const bool convex = ccw ? turn > 0 : turn < 0;
            if (!convex) continue;

            bool anyInside = false;
            for (size_t k = 0; k < m && !anyInside; ++k) {
                if (k == iPrev || k == i || k == iNext) continue;
                if (pointInTriangle(ring[remaining[k]], a, b, c, ccw)) anyInside = true;
            }
            if (anyInside) continue;

            triangles.push_back({remaining[iPrev], remaining[i], remaining[iNext]});
            remaining.erase(remaining.begin() + static_cast<long>(i));
            clipped = true;
            break;
        }
        if (!clipped) break;
    }
    if (remaining.size() == 3) triangles.push_back({remaining[0], remaining[1], remaining[2]});
    return triangles;
}

// Ports MapLibre core's roundPolygonCorners() (src/mbgl/tile/geometry_tile_data.cpp) to operate on
// one open ring (no repeated closing point) of plain 2D points. Same algorithm: clip each corner
// by up to `cornerDistance` (capped to 20% of either adjacent edge, so short edges never overlap)
// and replace it with a short circular arc.
std::vector<Point2D> roundRingCorners(const std::vector<Point2D>& ring, double cornerDistance) {
    constexpr int arcPoints = 3;
    constexpr double maxEdgeLenPercent = 0.2;
    const double sinParallelThreshold = std::sin(5.0 * M_PI / 180.0);
    const size_t n = ring.size();
    if (n < 3 || cornerDistance <= 0) return ring;

    std::vector<Point2D> rounded;
    rounded.reserve(n * (2 + arcPoints));
    for (size_t i = 0; i < n; ++i) {
        const auto& prevPoint = ring[(i + n - 1) % n];
        const auto& cornerPoint = ring[i];
        const auto& nextPoint = ring[(i + 1) % n];

        const auto edge1 = normalized(cornerPoint - prevPoint);
        const auto edge2 = normalized(nextPoint - cornerPoint);
        const double edge1Max = distance(cornerPoint, prevPoint) * maxEdgeLenPercent;
        const double edge2Max = distance(cornerPoint, nextPoint) * maxEdgeLenPercent;
        const double distanceHere = std::min({cornerDistance, edge1Max, edge2Max});

        const auto startPoint = cornerPoint - edge1 * distanceHere;
        const auto endPoint = cornerPoint + edge2 * distanceHere;

        auto perp1 = perp(edge1);
        auto perp2 = perp(edge2);
        if (cross(edge1, edge2) < 0) {
            perp1 = perp1 * -1.0;
            perp2 = perp2 * -1.0;
        }
        const double perpCross = cross(perp1, perp2);
        if (std::abs(perpCross) < sinParallelThreshold) {
            rounded.push_back(cornerPoint);
            continue;
        }
        const double t = cross(endPoint - startPoint, perp2) / perpCross;
        const auto center = startPoint + perp1 * t;

        rounded.push_back(startPoint);
        const double radius = distance(startPoint, center);
        const double startAngle = std::atan2(startPoint.y - center.y, startPoint.x - center.x);
        const double arcAngle = angleBetween(startPoint - center, endPoint - center);
        for (int k = 1; k <= arcPoints; ++k) {
            const double angle = startAngle + arcAngle * k / (arcPoints + 1);
            rounded.push_back({center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius});
        }
        rounded.push_back(endPoint);
    }
    return rounded;
}

// Splices a hole ring into a simple (outer) ring via a zero-width bridge, the standard technique
// for triangulating a polygon-with-holes using plain ear-clipping: connect the hole's closest
// vertex to the outer ring's closest vertex, duplicating both to create a degenerate "cut" that
// ear-clipping naturally absorbs without producing spurious triangles. This uses a simplified
// nearest-vertex bridge (no full visibility check), which is not bulletproof for pathological
// self-near-touching shapes but is standard and sufficient for realistic building footprints
// (e.g. a rectangular courtyard). Assumes the hole is already wound opposite the outer ring, the
// universal GeoJSON/vector-tile convention -- the same assumption the per-ring wall/outward-normal
// code elsewhere in this file already relies on.
std::vector<Point2D> bridgeHoleIntoRing(const std::vector<Point2D>& outer, const std::vector<Point2D>& hole) {
    if (hole.size() < 3 || outer.size() < 3) return outer;
    size_t bestOuter = 0, bestHole = 0;
    double bestDistSq = std::numeric_limits<double>::max();
    for (size_t i = 0; i < outer.size(); ++i) {
        for (size_t j = 0; j < hole.size(); ++j) {
            const auto d = outer[i] - hole[j];
            const double distSq = d.x * d.x + d.y * d.y;
            if (distSq < bestDistSq) {
                bestDistSq = distSq;
                bestOuter = i;
                bestHole = j;
            }
        }
    }
    std::vector<Point2D> merged;
    merged.reserve(outer.size() + hole.size() + 2);
    for (size_t i = 0; i <= bestOuter; ++i) merged.push_back(outer[i]);
    for (size_t k = 0; k < hole.size(); ++k) merged.push_back(hole[(bestHole + k) % hole.size()]);
    merged.push_back(hole[bestHole]);
    merged.push_back(outer[bestOuter]);
    for (size_t i = bestOuter + 1; i < outer.size(); ++i) merged.push_back(outer[i]);
    return merged;
}

mln_plugin_status createLayout(const mln_plugin_layout_context_v1* context, void** instance) try {
    if (!context || context->struct_size < sizeof(*context) || !instance) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    auto layout = std::unique_ptr<Layout>(new (std::nothrow) Layout());
    if (!layout) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
    // Index 0 matches layoutProperties[0] (fill-extrusion-plugin-rounded-corner-distance) --
    // the only layout property this layer type declares.
    if (context->layout_property_value_count > 0 && context->layout_property_values &&
        context->layout_property_values[0].type == MLN_PLUGIN_VALUE_FLOAT) {
        layout->roundedCornerDistance = context->layout_property_values[0].data.float_value;
    }
    startSegment(*layout);
    *instance = layout.release();
    return MLN_PLUGIN_STATUS_OK;
} catch (...) {
    return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
}

mln_plugin_status layoutFeature(void* instance, const mln_plugin_feature_v1* feature) try {
    if (!instance || !feature || feature->struct_size < sizeof(*feature) ||
        feature->geometry_type != MLN_PLUGIN_GEOMETRY_POLYGON || !feature->points || !feature->path_offsets ||
        feature->path_count == 0) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    auto& layout = *static_cast<Layout*>(instance);
    const auto firstVertex = static_cast<uint32_t>(layout.vertices.size());

    const auto clampCoord = [](double v) { return static_cast<int16_t>(std::clamp(v, -32768.0, 32767.0)); };
    const auto ringWinding = [](const std::vector<Point2D>& ring2D) {
        double signedArea = 0;
        for (size_t i = 0; i < ring2D.size(); ++i) {
            const auto& a = ring2D[i];
            const auto& b = ring2D[(i + 1) % ring2D.size()];
            signedArea += (a.x * b.y - b.x * a.y);
        }
        return signedArea > 0;
    };

    // Every ring (outer at index 0, any holes after) gets its own rounded corners. Real vector
    // tiles/GeoJSON wind holes opposite the outer ring; the wall generation below relies on that
    // convention (its outward-normal formula is entirely derived from each ring's own winding),
    // so hole walls come out correctly oriented with no special-casing needed.
    std::vector<std::vector<Point2D>> rings;
    for (size_t pathIndex = 0; pathIndex < feature->path_count; ++pathIndex) {
        const uint32_t ringStart = feature->path_offsets[pathIndex];
        const uint32_t ringEnd = feature->path_offsets[pathIndex + 1];
        std::vector<mln_plugin_tile_point_v1> raw;
        if (ringEnd > ringStart) raw.assign(feature->points + ringStart, feature->points + ringEnd);
        if (raw.size() > 1 && raw.front().x == raw.back().x && raw.front().y == raw.back().y) raw.pop_back();
        if (raw.size() < 3) continue;

        std::vector<Point2D> ring2D;
        ring2D.reserve(raw.size());
        for (const auto& p : raw) ring2D.push_back({static_cast<double>(p.x), static_cast<double>(p.y)});
        rings.push_back(roundRingCorners(ring2D, layout.roundedCornerDistance));
    }
    if (rings.empty()) return MLN_PLUGIN_STATUS_OK;

    // Wall quads for every ring (outer and holes alike): base ring vertex (z = base) up to top
    // ring vertex (z = height); the shader decides what "base"/"height" mean (a real Z lift for
    // the building, a ground shear for the shadow) -- the CPU-side mesh is identical either way.
    // Each edge also gets its outward 2D normal (building shader only) for basic wall shading.
    for (const auto& ring2D : rings) {
        std::vector<Vertex> ring;
        ring.reserve(ring2D.size());
        for (const auto& p : ring2D) ring.push_back({clampCoord(p.x), clampCoord(p.y), 0, 0, 0});

        const bool ccw = ringWinding(ring2D);
        for (size_t i = 0; i < ring.size(); ++i) {
            const auto p0 = ring[i];
            const auto p1 = ring[(i + 1) % ring.size()];
            const auto edge =
                normalized(Point2D{static_cast<double>(p1.x - p0.x), static_cast<double>(p1.y - p0.y)});
            const auto outward = ccw ? Point2D{edge.y, -edge.x} : Point2D{-edge.y, edge.x};
            const auto nx = static_cast<int16_t>(std::clamp(outward.x * kNormalScale, -32768.0, 32767.0));
            const auto ny = static_cast<int16_t>(std::clamp(outward.y * kNormalScale, -32768.0, 32767.0));
            // The outward normal formula above already compensates for the ring's own winding
            // direction, but the quad's vertex order does not by itself -- viewed from the
            // outward side, this quad is CCW (front-facing under back-face culling) only when
            // the ring is wound ccw; a cw ring needs the two ring-order vertices swapped to stay
            // CCW-outward too, otherwise cw-wound rings would render inside-out (or get culled)
            // once back-face culling is enabled.
            if (ccw) {
                pushQuad(layout,
                        Vertex{p0.x, p0.y, 0, nx, ny},
                        Vertex{p1.x, p1.y, 0, nx, ny},
                        Vertex{p1.x, p1.y, 1, nx, ny},
                        Vertex{p0.x, p0.y, 1, nx, ny});
            } else {
                pushQuad(layout,
                        Vertex{p1.x, p1.y, 0, nx, ny},
                        Vertex{p0.x, p0.y, 0, nx, ny},
                        Vertex{p0.x, p0.y, 1, nx, ny},
                        Vertex{p1.x, p1.y, 1, nx, ny});
            }
        }
    }

    // Roof cap: bridge every hole into the outer ring to form one simple polygon (the standard
    // technique for triangulating a polygon-with-holes via plain ear-clipping -- see
    // bridgeHoleIntoRing), then triangulate the combined ring as a whole (this also closes the
    // far end of the shadow shader's ground sweep).
    std::vector<Point2D> roofRing = rings[0];
    for (size_t holeIndex = 1; holeIndex < rings.size(); ++holeIndex) {
        roofRing = bridgeHoleIntoRing(roofRing, rings[holeIndex]);
    }
    const bool outerCcw = ringWinding(rings[0]);
    std::vector<Vertex> roofVertices;
    roofVertices.reserve(roofRing.size());
    // Roof cap vertices keep the (0, 0) normal sentinel -- the shader treats that as "up".
    for (const auto& p : roofRing) roofVertices.push_back({clampCoord(p.x), clampCoord(p.y), 1, 0, 0});
    const auto roofTriangles = triangulateRing(roofRing);
    for (const auto& tri : roofTriangles) {
        const auto a = roofVertices[tri[0]];
        const auto b = roofVertices[tri[1]];
        const auto c = roofVertices[tri[2]];
        // Ear-clipping preserves the combined ring's own winding, so -- same reasoning as the
        // wall quads above -- a cw ring needs its triangles' last two vertices swapped to stay
        // CCW when viewed from above (the roof's outward/"up" direction).
        if (outerCcw) {
            pushTriangle(layout, a, b, c);
        } else {
            pushTriangle(layout, a, c, b);
        }
    }

    if (layout.vertices.size() > firstVertex) {
        const auto count = static_cast<uint32_t>(layout.vertices.size() - firstVertex);
        layout.featureRanges.push_back(
            {sizeof(mln_plugin_feature_vertex_range_v1), feature->feature_index, buildingDrawableKey, firstVertex, count});
        layout.featureRanges.push_back(
            {sizeof(mln_plugin_feature_vertex_range_v1), feature->feature_index, shadowDrawableKey, firstVertex, count});
    }
    return MLN_PLUGIN_STATUS_OK;
} catch (...) {
    return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
}

mln_plugin_status finishLayout(void* instance, mln_plugin_bucket_v1* output) {
    if (!instance || !output || output->struct_size < sizeof(*output)) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    auto& layout = *static_cast<Layout*>(instance);
    layout.segments.erase(std::remove_if(layout.segments.begin(),
                                         layout.segments.end(),
                                         [](const auto& segment) { return segment.index_length == 0; }),
                          layout.segments.end());
    layout.streams[0] = {sizeof(mln_plugin_vertex_stream_v1),
                         vertexStream,
                         reinterpret_cast<const uint8_t*>(layout.vertices.data()),
                         layout.vertices.size() * sizeof(Vertex),
                         static_cast<uint32_t>(layout.vertices.size()),
                         sizeof(Vertex)};
    // The host requires attribute_count (per drawable) + that shader's own property-bound
    // attribute count to equal the shader's total declared attributes -- so each drawable gets
    // only the real (non-property) attributes its own shader actually declares.
    layout.buildingAttributes = {{
        {sizeof(mln_plugin_attribute_binding_v1), positionAttribute, vertexStream, offsetof(Vertex, x)},
        {sizeof(mln_plugin_attribute_binding_v1), topAttribute, vertexStream, offsetof(Vertex, top)},
        {sizeof(mln_plugin_attribute_binding_v1), normalAttribute, vertexStream, offsetof(Vertex, nx)},
    }};
    layout.shadowAttributes = {{
        {sizeof(mln_plugin_attribute_binding_v1), positionAttribute, vertexStream, offsetof(Vertex, x)},
        {sizeof(mln_plugin_attribute_binding_v1), topAttribute, vertexStream, offsetof(Vertex, top)},
    }};

    // Both drawables share the exact same vertex/index geometry and segments -- only the shader
    // (and therefore how "top ? height : base" gets turned into a screen position) differs.
    // Shadow is listed first (drawn first) and building second (drawn on top): both are
    // translucent with no depth test (stencil overlap dedup instead -- see the enable_depth_write
    // doc comment), so whichever draws last wins any screen-space overlap. Shadow's own ground
    // sweep always touches the building's own base/wall footprint by construction (its near edge
    // is the building's own unshifted footprint), so drawing building second ensures its own
    // color covers that overlap instead of the shadow tinting a visible notch into the building.
    auto& shadowDrawable = layout.drawables[0];
    shadowDrawable.struct_size = sizeof(shadowDrawable);
    shadowDrawable.drawable_key = shadowDrawableKey;
    shadowDrawable.shader_id = str("shadow");
    shadowDrawable.attributes = layout.shadowAttributes.data();
    shadowDrawable.attribute_count = layout.shadowAttributes.size();
    shadowDrawable.segments = layout.segments.data();
    shadowDrawable.segment_count = layout.segments.size();

    auto& buildingDrawable = layout.drawables[1];
    buildingDrawable.struct_size = sizeof(buildingDrawable);
    buildingDrawable.drawable_key = buildingDrawableKey;
    buildingDrawable.shader_id = str("building");
    buildingDrawable.attributes = layout.buildingAttributes.data();
    buildingDrawable.attribute_count = layout.buildingAttributes.size();
    buildingDrawable.segments = layout.segments.data();
    buildingDrawable.segment_count = layout.segments.size();

    const bool hasGeometry = !layout.indices.empty();
    output->vertex_streams = layout.vertices.empty() ? nullptr : layout.streams.data();
    output->vertex_stream_count = layout.vertices.empty() ? 0 : layout.streams.size();
    output->indices = layout.indices.data();
    output->index_count = layout.indices.size();
    output->drawables = hasGeometry ? layout.drawables.data() : nullptr;
    output->drawable_count = hasGeometry ? layout.drawables.size() : 0;
    output->query_radius = 0.0f;
    output->feature_vertex_ranges = layout.featureRanges.data();
    output->feature_vertex_range_count = layout.featureRanges.size();
    return MLN_PLUGIN_STATUS_OK;
}

void destroyLayout(void* instance) {
    delete static_cast<Layout*>(instance);
}

struct alignas(16) BuildingDrawableUBO {
    float matrix[16];
    float height_t;
    float base_t;
    float color_t;
    float opacity_t;
    float vertical_gradient_t;
    float pad0, pad1, pad2;
};
static_assert(sizeof(BuildingDrawableUBO) == 96);

struct alignas(16) BuildingLayerUBO {
    float color[4];
    float opacity;
    float height;
    float base;
    float vertical_gradient; // property-bound (dead attribute), 0 or 1
    // Not properties -- written directly by updateUniformBlock() from the render context, never
    // touched by the property-binding writer (see the enable_near_clipped_matrix-style plumbing
    // in plugin_layer_tweaker.cpp that populates mln_plugin_uniform_context_v1's light fields).
    float light_color[3];
    float pad0;
    float light_position[3];
    float light_intensity;
};
static_assert(sizeof(BuildingLayerUBO) == 64);

struct alignas(16) ShadowDrawableUBO {
    float matrix[16];
    float azimuth_t;
    float length_t;
    float opacity_t;
    float height_t;
    float base_t;
    float color_t;
    float pad0, pad1;
};
static_assert(sizeof(ShadowDrawableUBO) == 96);

struct alignas(16) ShadowLayerUBO {
    float color[4];
    float opacity;
    float azimuth;
    float length_;
    float height;
    float base;
    float pad0, pad1, pad2;
};
static_assert(sizeof(ShadowLayerUBO) == 48);

mln_plugin_status updateUniformBlock(const mln_plugin_uniform_context_v1* context,
                                     uint32_t id,
                                     uint8_t* output,
                                     size_t size) {
    if (!context || context->struct_size < sizeof(*context) || !output) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    if (id == buildingDrawableUniformId || id == shadowDrawableUniformId) {
        const size_t expected = id == buildingDrawableUniformId ? sizeof(BuildingDrawableUBO) : sizeof(ShadowDrawableUBO);
        if (size != expected) return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        std::memset(output, 0, size);
        std::memcpy(output, context->tile_matrix, sizeof(context->tile_matrix));
        return MLN_PLUGIN_STATUS_OK;
    }
    if (id == buildingLayerUniformId) {
        if (size != sizeof(BuildingLayerUBO)) return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        BuildingLayerUBO value{};
        std::memcpy(value.light_color, context->light_color, sizeof(value.light_color));
        std::memcpy(value.light_position, context->light_position, sizeof(value.light_position));
        value.light_intensity = context->light_intensity;
        std::memcpy(output, &value, sizeof(value));
        return MLN_PLUGIN_STATUS_OK;
    }
    if (id == shadowLayerUniformId) {
        return size == sizeof(ShadowLayerUBO) ? MLN_PLUGIN_STATUS_OK : MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
}

constexpr mln_plugin_value number(float n) {
    mln_plugin_value v{};
    v.struct_size = sizeof(v);
    v.type = MLN_PLUGIN_VALUE_FLOAT;
    v.data.float_value = n;
    return v;
}
constexpr mln_plugin_value opaqueColor(float r, float g, float b) {
    mln_plugin_value v{};
    v.struct_size = sizeof(v);
    v.type = MLN_PLUGIN_VALUE_COLOR;
    v.data.color_value = {r, g, b, 1};
    return v;
}
constexpr mln_plugin_value blackColor() {
    return opaqueColor(0, 0, 0);
}
constexpr mln_plugin_value float2(float x, float y) {
    mln_plugin_value v{};
    v.struct_size = sizeof(v);
    v.type = MLN_PLUGIN_VALUE_FLOAT2;
    v.data.float2_value = {x, y};
    return v;
}
constexpr mln_plugin_value stringValue(mln_plugin_string s) {
    mln_plugin_value v{};
    v.struct_size = sizeof(v);
    v.type = MLN_PLUGIN_VALUE_STRING;
    v.data.string_value = s;
    return v;
}

constexpr mln_plugin_property_descriptor_v1 property(mln_plugin_string name,
                                                      mln_plugin_value value,
                                                      float minimum = -std::numeric_limits<float>::infinity(),
                                                      float maximum = std::numeric_limits<float>::infinity(),
                                                      uint32_t expressionCapabilities = MLN_PLUGIN_EXPRESSION_CAMERA) {
    mln_plugin_property_descriptor_v1 p{};
    p.struct_size = sizeof(p);
    p.name = name;
    p.type = value.type;
    p.default_value = value;
    p.expression_capabilities = expressionCapabilities;
    p.supports_transitions = 1;
    p.has_minimum = minimum != -std::numeric_limits<float>::infinity();
    p.minimum = p.has_minimum ? minimum : 0;
    p.has_maximum = maximum != std::numeric_limits<float>::infinity();
    p.maximum = p.has_maximum ? maximum : 0;
    return p;
}

// String-enum, camera-only paint property (e.g. translate-anchor's "map"/"viewport") -- matches
// real style syntax exactly, unlike encoding a 0/1 float, so styles ported from real
// fill-extrusion by only renaming property keys (not rewriting values) still parse correctly.
constexpr mln_plugin_property_descriptor_v1 stringEnumProperty(mln_plugin_string name,
                                                                mln_plugin_string defaultValue,
                                                                const mln_plugin_string* enumValues,
                                                                size_t enumValueCount) {
    mln_plugin_property_descriptor_v1 p{};
    p.struct_size = sizeof(p);
    p.name = name;
    p.type = MLN_PLUGIN_VALUE_STRING;
    p.default_value = stringValue(defaultValue);
    p.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA;
    p.supports_transitions = 0;
    p.enum_values = enumValues;
    p.enum_value_count = enumValueCount;
    return p;
}

// Layout properties are evaluated once per bucket at layout time, not per-frame, so unlike
// `property()` they never support transitions or anything beyond camera/zoom expressions --
// the host rejects a descriptor claiming otherwise (see plugin_registry.cpp).
constexpr mln_plugin_property_descriptor_v1 layoutProperty(
    mln_plugin_string name,
    mln_plugin_value value,
    float minimum = -std::numeric_limits<float>::infinity(),
    float maximum = std::numeric_limits<float>::infinity()) {
    mln_plugin_property_descriptor_v1 p{};
    p.struct_size = sizeof(p);
    p.name = name;
    p.type = value.type;
    p.default_value = value;
    p.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA;
    p.supports_transitions = 0;
    p.has_minimum = minimum != -std::numeric_limits<float>::infinity();
    p.minimum = p.has_minimum ? minimum : 0;
    p.has_maximum = maximum != std::numeric_limits<float>::infinity();
    p.maximum = p.has_maximum ? maximum : 0;
    return p;
}

constexpr uint32_t dataDrivenCapabilities = MLN_PLUGIN_EXPRESSION_CAMERA | MLN_PLUGIN_EXPRESSION_FEATURE |
                                            MLN_PLUGIN_EXPRESSION_COMPOSITE;

const mln_plugin_string translateAnchorValues[] = {str("map"), str("viewport")};

const mln_plugin_property_descriptor_v1 properties[] = {
    property(str("fill-extrusion-plugin-color"), opaqueColor(0.79f, 0.8f, 0.82f), -INFINITY, INFINITY,
            dataDrivenCapabilities),
    property(str("fill-extrusion-plugin-opacity"), number(1), 0, 1),
    property(str("fill-extrusion-plugin-height"), number(0), 0, INFINITY, dataDrivenCapabilities),
    property(str("fill-extrusion-plugin-base"), number(0), 0, INFINITY, dataDrivenCapabilities),
    property(str("fill-extrusion-plugin-shadow-color"), blackColor()),
    property(str("fill-extrusion-plugin-shadow-opacity"), number(0.10f), 0, 1),
    property(str("fill-extrusion-plugin-shadow-azimuth"), number(225)),
    property(str("fill-extrusion-plugin-shadow-length"), number(0.5f), 0),
    // 0 or 1: there is no dedicated boolean value type in the plugin ABI.
    property(str("fill-extrusion-plugin-vertical-gradient"), number(1), 0, 1),
    // Camera-only, read directly by the tweaker (PluginLayerTweaker::execute) and applied via
    // getTileMatrix()'s translation/anchor parameters -- not property-bound to any shader
    // uniform, since it affects the tile matrix itself rather than per-vertex/per-fragment math.
    property(str("fill-extrusion-plugin-translate"), float2(0, 0)),
    stringEnumProperty(str("fill-extrusion-plugin-translate-anchor"), str("map"), translateAnchorValues,
                       std::size(translateAnchorValues)),
};

const mln_plugin_property_descriptor_v1 layoutProperties[] = {
    // Read once per bucket in createLayout() and applied to the rounded ring before
    // triangulation, matching real fill-extrusion-rounded-corner-distance (a LayoutProperty<float>
    // in core, evaluated once per bucket the same way).
    layoutProperty(str("fill-extrusion-plugin-rounded-corner-distance"), number(kDefaultRoundedCornerDistance), 0),
};

const mln_plugin_shader_property_binding_v1 buildingPropertyBindings[] = {
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-color"),
     MLN_PLUGIN_PROPERTY_ENCODING_COLOR,
     buildingLayerUniformId,
     offsetof(BuildingLayerUBO, color),
     buildingColorMinAttribute,
     buildingColorMaxAttribute,
     buildingDrawableUniformId,
     offsetof(BuildingDrawableUBO, color_t)},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-opacity"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     buildingLayerUniformId,
     offsetof(BuildingLayerUBO, opacity),
     buildingOpacityAttribute,
     buildingOpacityAttribute,
     buildingDrawableUniformId,
     offsetof(BuildingDrawableUBO, opacity_t)},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-height"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     buildingLayerUniformId,
     offsetof(BuildingLayerUBO, height),
     heightAttribute,
     heightAttribute,
     buildingDrawableUniformId,
     offsetof(BuildingDrawableUBO, height_t)},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-base"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     buildingLayerUniformId,
     offsetof(BuildingLayerUBO, base),
     baseAttribute,
     baseAttribute,
     buildingDrawableUniformId,
     offsetof(BuildingDrawableUBO, base_t)},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-vertical-gradient"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     buildingLayerUniformId,
     offsetof(BuildingLayerUBO, vertical_gradient),
     verticalGradientAttribute,
     verticalGradientAttribute,
     buildingDrawableUniformId,
     offsetof(BuildingDrawableUBO, vertical_gradient_t)},
};

const mln_plugin_shader_property_binding_v1 shadowPropertyBindings[] = {
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-shadow-color"),
     MLN_PLUGIN_PROPERTY_ENCODING_COLOR,
     shadowLayerUniformId,
     offsetof(ShadowLayerUBO, color),
     shadowColorMinAttribute,
     shadowColorMaxAttribute,
     shadowDrawableUniformId,
     offsetof(ShadowDrawableUBO, color_t)},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-shadow-opacity"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     shadowLayerUniformId,
     offsetof(ShadowLayerUBO, opacity),
     shadowOpacityAttribute,
     shadowOpacityAttribute,
     shadowDrawableUniformId,
     offsetof(ShadowDrawableUBO, opacity_t)},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-shadow-azimuth"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     shadowLayerUniformId,
     offsetof(ShadowLayerUBO, azimuth),
     shadowAzimuthAttribute,
     shadowAzimuthAttribute,
     shadowDrawableUniformId,
     offsetof(ShadowDrawableUBO, azimuth_t)},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-shadow-length"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     shadowLayerUniformId,
     offsetof(ShadowLayerUBO, length_),
     shadowLengthAttribute,
     shadowLengthAttribute,
     shadowDrawableUniformId,
     offsetof(ShadowDrawableUBO, length_t)},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-height"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     shadowLayerUniformId,
     offsetof(ShadowLayerUBO, height),
     heightAttribute,
     heightAttribute,
     shadowDrawableUniformId,
     offsetof(ShadowDrawableUBO, height_t)},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-plugin-base"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     shadowLayerUniformId,
     offsetof(ShadowLayerUBO, base),
     baseAttribute,
     baseAttribute,
     shadowDrawableUniformId,
     offsetof(ShadowDrawableUBO, base_t)},
};

constexpr char buildingOpenglVertex[] = R"SHADER(
in vec2 a_position;
in float a_top;
in vec2 a_normal;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_HEIGHT_IS_UNIFORM
in vec2 a_height;
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_BASE_IS_UNIFORM
in vec2 a_base;
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_COLOR_IS_UNIFORM
in vec4 a_color_min;
in vec4 a_color_max;
#endif

layout (std140) uniform BuildingDrawableUBO {
    mat4 u_matrix;
    float u_height_t, u_base_t, u_color_t, u_opacity_t, u_vertical_gradient_t;
};
layout (std140) uniform BuildingLayerUBO {
    vec4 u_color;
    float u_opacity;
    float u_height;
    float u_base;
    float u_vertical_gradient;
    vec3 u_light_color;
    vec3 u_light_position;
    float u_light_intensity;
};

out vec4 v_color;

void main() {
    float height = u_height;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_HEIGHT_IS_UNIFORM
    height = mix(a_height.x, a_height.y, u_height_t);
#endif
    float base = u_base;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_BASE_IS_UNIFORM
    base = mix(a_base.x, a_base.y, u_base_t);
#endif
    vec4 color = u_color;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_COLOR_IS_UNIFORM
    color = mix(a_color_min, a_color_max, u_color_t);
#endif
    base = max(0.0, base);
    height = max(0.0, height);
    float z = a_top > 0.5 ? height : base;
    gl_Position = u_matrix * vec4(a_position, z, 1.0);

    // Ported from real fill-extrusion's own lighting formula (shaders/fill_extrusion.vertex.glsl),
    // computed here in the vertex stage exactly as core does, rather than approximated. (0, 0) is
    // the roof-cap sentinel for "straight up"; anything else is a wall's horizontal outward normal,
    // packed as int16 scaled by 32767 (core scales by 16384 for its own packing; the constant is
    // purely an internal encoding detail and doesn't need to match).
    vec3 normal = length(a_normal) < 0.5 ? vec3(0.0, 0.0, 1.0) : vec3(a_normal / 32767.0, 0.0);

    // Relative luminance (how dark/bright is the surface color?), before ambient is added.
    float colorvalue = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
    // Slight ambient lighting so no extrusions are totally black.
    color.rgb += vec3(0.03);

    // cos(theta) between surface normal and diffuse light ray. u_light_position is intentionally
    // not normalized here -- its magnitude (the style's light "radial distance") affects the
    // dot product on purpose, matching core exactly.
    float directional = clamp(dot(normal, u_light_position), 0.0, 1.0);
    // Narrow the highlight/shading range for lower light intensity and brighter surface colors.
    directional = mix(1.0 - u_light_intensity, max(1.0 - colorvalue + u_light_intensity, 1.0), directional);

    // Gradient along z axis of side surfaces only (roof's sentinel normal has z == 1.0).
    if (normal.z == 0.0) {
        float fMin = mix(0.7, 0.98, 1.0 - u_light_intensity);
        float factor = clamp((a_top + base) * pow(height / 150.0, 0.5), fMin, 1.0);
        directional *= (1.0 - u_vertical_gradient) + (u_vertical_gradient * factor);
    }

    // Final color: surface + ambient, times diffuse directional, times light color, with a lower
    // bound tinted toward the complementary color of the light.
    vec3 lit = clamp(color.rgb * directional * u_light_color,
                     mix(vec3(0.0), vec3(0.3), 1.0 - u_light_color),
                     vec3(1.0));
    v_color = vec4(lit, 1.0) * u_opacity;
}
)SHADER";

constexpr char buildingOpenglFragment[] = R"SHADER(
in vec4 v_color;

void main() {
    fragColor = v_color;
}
)SHADER";

constexpr char buildingMetalSource[] = R"SHADER(
struct BuildingVertex {
    short2 a_position [[attribute(0)]];
    ushort a_top [[attribute(1)]];
    short2 a_normal [[attribute(7)]];
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_HEIGHT_IS_UNIFORM
    float2 a_height [[attribute(4)]];
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_BASE_IS_UNIFORM
    float2 a_base [[attribute(5)]];
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_COLOR_IS_UNIFORM
    float4 a_color_min [[attribute(2)]];
    float4 a_color_max [[attribute(3)]];
#endif
};
struct alignas(16) BuildingDrawableUBO {
    float4x4 matrix;
    float height_t, base_t, color_t, opacity_t, vertical_gradient_t;
    float pad0, pad1, pad2;
};
struct alignas(16) BuildingLayerUBO {
    float4 color;
    float opacity;
    float height;
    float base;
    float vertical_gradient;
    float3 light_color;
    float pad0;
    float3 light_position;
    float light_intensity;
};
struct BuildingVaryings {
    float4 position [[position]];
    float4 color;
};

vertex BuildingVaryings buildingVertex(BuildingVertex in [[stage_in]],
    constant BuildingDrawableUBO& drawable [[buffer(MLN_PLUGIN_UNIFORM_0_BINDING)]],
    constant BuildingLayerUBO& layer [[buffer(MLN_PLUGIN_UNIFORM_1_BINDING)]]) {
    BuildingVaryings out;
    float height = layer.height;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_HEIGHT_IS_UNIFORM
    height = mix(in.a_height.x, in.a_height.y, drawable.height_t);
#endif
    float base = layer.base;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_BASE_IS_UNIFORM
    base = mix(in.a_base.x, in.a_base.y, drawable.base_t);
#endif
    float4 color = layer.color;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_COLOR_IS_UNIFORM
    color = mix(in.a_color_min, in.a_color_max, drawable.color_t);
#endif
    base = max(0.0, base);
    height = max(0.0, height);
    float z = in.a_top > 0 ? height : base;
    out.position = drawable.matrix * float4(float2(in.a_position), z, 1.0);

    // Ported from real fill-extrusion's own lighting formula (shaders/fill_extrusion.vertex.glsl).
    float2 normal2 = float2(in.a_normal) / 32767.0;
    float3 normal = length(normal2) < 0.5 ? float3(0.0, 0.0, 1.0) : float3(normal2, 0.0);

    float colorvalue = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
    color.rgb += float3(0.03);

    // layer.light_position is intentionally not normalized -- its magnitude (the style's light
    // "radial distance") affects the dot product on purpose, matching core exactly.
    float directional = clamp(dot(normal, layer.light_position), 0.0, 1.0);
    directional = mix(1.0 - layer.light_intensity, fmax(1.0 - colorvalue + layer.light_intensity, 1.0), directional);

    if (normal.z == 0.0) {
        float fMin = mix(0.7, 0.98, 1.0 - layer.light_intensity);
        float factor = clamp((float(in.a_top) + base) * pow(height / 150.0, 0.5), fMin, 1.0);
        directional *= (1.0 - layer.vertical_gradient) + (layer.vertical_gradient * factor);
    }

    float3 lit = clamp(color.rgb * directional * layer.light_color,
                       mix(float3(0.0), float3(0.3), 1.0 - layer.light_color),
                       float3(1.0));
    out.color = float4(lit, 1.0) * layer.opacity;
    return out;
}
fragment half4 buildingFragment(BuildingVaryings in [[stage_in]]) {
    return half4(in.color);
}
)SHADER";

constexpr char shadowOpenglVertex[] = R"SHADER(
in vec2 a_position;
in float a_top;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_HEIGHT_IS_UNIFORM
in vec2 a_height;
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_BASE_IS_UNIFORM
in vec2 a_base;
#endif

layout (std140) uniform ShadowDrawableUBO {
    mat4 u_matrix;
    float u_azimuth_t, u_length_t, u_opacity_t, u_height_t, u_base_t, u_color_t;
};
layout (std140) uniform ShadowLayerUBO {
    vec4 u_color;
    float u_opacity;
    float u_azimuth;
    float u_length;
    float u_height;
    float u_base;
};

void main() {
    float height = u_height;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_HEIGHT_IS_UNIFORM
    height = mix(a_height.x, a_height.y, u_height_t);
#endif
    float base = u_base;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_BASE_IS_UNIFORM
    base = mix(a_base.x, a_base.y, u_base_t);
#endif
    float z = a_top > 0.5 ? height : base;
    float azimuthRad = radians(u_azimuth);
    vec2 direction = vec2(sin(azimuthRad), -cos(azimuthRad));
    vec2 offset = direction * u_length * z;
    gl_Position = u_matrix * vec4(a_position + offset, 0.0, 1.0);
}
)SHADER";

constexpr char shadowOpenglFragment[] = R"SHADER(
layout (std140) uniform ShadowLayerUBO {
    vec4 u_color;
    float u_opacity;
    float u_azimuth;
    float u_length;
    float u_height;
    float u_base;
};

void main() {
    fragColor = u_color * u_opacity;
}
)SHADER";

constexpr char shadowMetalSource[] = R"SHADER(
struct ShadowVertex {
    short2 a_position [[attribute(0)]];
    ushort a_top [[attribute(1)]];
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_HEIGHT_IS_UNIFORM
    float2 a_height [[attribute(2)]];
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_BASE_IS_UNIFORM
    float2 a_base [[attribute(3)]];
#endif
};
struct alignas(16) ShadowDrawableUBO {
    float4x4 matrix;
    float azimuth_t, length_t, opacity_t, height_t, base_t, color_t, pad0, pad1;
};
struct alignas(16) ShadowLayerUBO {
    float4 color;
    float opacity;
    float azimuth;
    float length_;
    float height;
    float base;
    float pad0, pad1, pad2;
};
struct ShadowVaryings {
    float4 position [[position]];
};

vertex ShadowVaryings shadowVertex(ShadowVertex in [[stage_in]],
    constant ShadowDrawableUBO& drawable [[buffer(MLN_PLUGIN_UNIFORM_2_BINDING)]],
    constant ShadowLayerUBO& layer [[buffer(MLN_PLUGIN_UNIFORM_3_BINDING)]]) {
    ShadowVaryings out;
    float height = layer.height;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_HEIGHT_IS_UNIFORM
    height = mix(in.a_height.x, in.a_height.y, drawable.height_t);
#endif
    float base = layer.base;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PLUGIN_BASE_IS_UNIFORM
    base = mix(in.a_base.x, in.a_base.y, drawable.base_t);
#endif
    float z = in.a_top > 0 ? height : base;
    float azimuthRad = layer.azimuth * 0.017453292519943295;
    float2 direction = float2(sin(azimuthRad), -cos(azimuthRad));
    float2 offset = direction * layer.length_ * z;
    float2 pos = float2(in.a_position) + offset;
    out.position = drawable.matrix * float4(pos, 0.0, 1.0);
    return out;
}
fragment half4 shadowFragment(ShadowVaryings in [[stage_in]],
    constant ShadowLayerUBO& layer [[buffer(MLN_PLUGIN_UNIFORM_3_BINDING)]]) {
    return half4(layer.color * layer.opacity);
}
)SHADER";

// The struct's `location` field must be dense and start at zero within each shader (validated by
// the host), independent of `attribute_id`, which is just this layer type's own bookkeeping and
// may be sparse/shared across shaders. Metal's `[[attribute(N)]]` annotations above must match
// these `location` values, not the `attribute_id` values.
const mln_plugin_shader_attribute_v1 buildingShaderAttributes[] = {
    {sizeof(mln_plugin_shader_attribute_v1), positionAttribute, 0, str("a_position"), MLN_PLUGIN_VERTEX_INT16_X2},
    {sizeof(mln_plugin_shader_attribute_v1), topAttribute, 1, str("a_top"), MLN_PLUGIN_VERTEX_UINT16},
    {sizeof(mln_plugin_shader_attribute_v1), buildingColorMinAttribute, 2, str("a_color_min"), MLN_PLUGIN_VERTEX_FLOAT_X4},
    {sizeof(mln_plugin_shader_attribute_v1), buildingColorMaxAttribute, 3, str("a_color_max"), MLN_PLUGIN_VERTEX_FLOAT_X4},
    {sizeof(mln_plugin_shader_attribute_v1), heightAttribute, 4, str("a_height"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), baseAttribute, 5, str("a_base"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), buildingOpacityAttribute, 6, str("a_opacity"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), normalAttribute, 7, str("a_normal"), MLN_PLUGIN_VERTEX_INT16_X2},
    {sizeof(mln_plugin_shader_attribute_v1), verticalGradientAttribute, 8, str("a_vertical_gradient"), MLN_PLUGIN_VERTEX_FLOAT_X2},
};

const mln_plugin_shader_attribute_v1 shadowShaderAttributes[] = {
    {sizeof(mln_plugin_shader_attribute_v1), positionAttribute, 0, str("a_position"), MLN_PLUGIN_VERTEX_INT16_X2},
    {sizeof(mln_plugin_shader_attribute_v1), topAttribute, 1, str("a_top"), MLN_PLUGIN_VERTEX_UINT16},
    {sizeof(mln_plugin_shader_attribute_v1), heightAttribute, 2, str("a_height"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), baseAttribute, 3, str("a_base"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), shadowAzimuthAttribute, 4, str("a_azimuth"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), shadowLengthAttribute, 5, str("a_length"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), shadowOpacityAttribute, 6, str("a_opacity"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), shadowColorMinAttribute, 7, str("a_color_min"), MLN_PLUGIN_VERTEX_FLOAT_X4},
    {sizeof(mln_plugin_shader_attribute_v1), shadowColorMaxAttribute, 8, str("a_color_max"), MLN_PLUGIN_VERTEX_FLOAT_X4},
};

const mln_plugin_uniform_block_descriptor_v1 buildingUniforms[] = {
    {sizeof(mln_plugin_uniform_block_descriptor_v1), buildingDrawableUniformId, str("BuildingDrawableUBO"),
     sizeof(BuildingDrawableUBO), MLN_PLUGIN_SHADER_STAGE_VERTEX, MLN_PLUGIN_UNIFORM_DRAWABLE},
    // Declared for both stages even though the fragment shader no longer reads it (all
    // lighting/color math now happens in the vertex stage, matching real fill-extrusion's own
    // shader) -- a vertex-only mask here would share an identical stage mask with the
    // drawable-scope BuildingDrawableUBO above, which the host rejects as two uniform blocks
    // packed into the same stage slot.
    {sizeof(mln_plugin_uniform_block_descriptor_v1), buildingLayerUniformId, str("BuildingLayerUBO"),
     sizeof(BuildingLayerUBO), MLN_PLUGIN_SHADER_STAGE_VERTEX | MLN_PLUGIN_SHADER_STAGE_FRAGMENT, MLN_PLUGIN_UNIFORM_LAYER},
};

const mln_plugin_uniform_block_descriptor_v1 shadowUniforms[] = {
    {sizeof(mln_plugin_uniform_block_descriptor_v1), shadowDrawableUniformId, str("ShadowDrawableUBO"),
     sizeof(ShadowDrawableUBO), MLN_PLUGIN_SHADER_STAGE_VERTEX, MLN_PLUGIN_UNIFORM_DRAWABLE},
    {sizeof(mln_plugin_uniform_block_descriptor_v1), shadowLayerUniformId, str("ShadowLayerUBO"),
     sizeof(ShadowLayerUBO), MLN_PLUGIN_SHADER_STAGE_VERTEX | MLN_PLUGIN_SHADER_STAGE_FRAGMENT, MLN_PLUGIN_UNIFORM_LAYER},
};

const mln_plugin_shader_source_v1 buildingShaderSources[] = {
    {sizeof(mln_plugin_shader_source_v1), MLN_PLUGIN_BACKEND_OPENGL, str(buildingOpenglVertex), str(buildingOpenglFragment), {}, {}},
    {sizeof(mln_plugin_shader_source_v1), MLN_PLUGIN_BACKEND_METAL, str(buildingMetalSource), {}, str("buildingVertex"), str("buildingFragment")},
};

const mln_plugin_shader_source_v1 shadowShaderSources[] = {
    {sizeof(mln_plugin_shader_source_v1), MLN_PLUGIN_BACKEND_OPENGL, str(shadowOpenglVertex), str(shadowOpenglFragment), {}, {}},
    {sizeof(mln_plugin_shader_source_v1), MLN_PLUGIN_BACKEND_METAL, str(shadowMetalSource), {}, str("shadowVertex"), str("shadowFragment")},
};

const mln_plugin_shader_descriptor_v1 buildingShader = {
    sizeof(mln_plugin_shader_descriptor_v1),
    str("building"),
    buildingShaderSources,
    std::size(buildingShaderSources),
    buildingShaderAttributes,
    std::size(buildingShaderAttributes),
    buildingUniforms,
    std::size(buildingUniforms),
    buildingPropertyBindings,
    std::size(buildingPropertyBindings),
    // Off (stencil overlap dedup instead, same as the shadow shader): real depth-write
    // (the two-pass depth pre-pass + color pass, see render_plugin_style_layer.cpp) genuinely
    // z-fights when two different features' surfaces land at nearly the same depth from the
    // camera -- confirmed via plugins/fill-extrusion-plugin/render-tests/core-parity's
    // fill-extrusion-height/function case (three overlapping boxes of different sizes/heights),
    // which renders with spiky z-fighting artifacts at enable_depth_write=1 but cleanly at 0.
    // This is a real floating-point depth-precision limitation, not a simple bug: real
    // fill-extrusion avoids it by encoding vertex positions with sub-pixel "decimals" precision
    // (see the tile-seam-gap limitation noted elsewhere in this file), which this plugin's plain
    // int16 x/y encoding doesn't have. Until that's implemented, stencil dedup is the stable
    // choice -- the tradeoff is losing real depth occlusion between separate buildings (relies on
    // style layer paint order only), which is deliberately accepted for now.
    0,
};

const mln_plugin_shader_descriptor_v1 shadowShader = {
    sizeof(mln_plugin_shader_descriptor_v1),
    str("shadow"),
    shadowShaderSources,
    std::size(shadowShaderSources),
    shadowShaderAttributes,
    std::size(shadowShaderAttributes),
    shadowUniforms,
    std::size(shadowUniforms),
    shadowPropertyBindings,
    std::size(shadowPropertyBindings),
    0, // enable_depth_write: off, this shader uses the layer-wide enable_stencil_overlap_dedup mode
};

const mln_plugin_shader_descriptor_v1 shaders[] = {buildingShader, shadowShader};

const mln_plugin_layer_type_v1 layerType = [] {
    mln_plugin_layer_type_v1 v{};
    v.struct_size = sizeof(v);
    v.layer_type = str("fill-extrusion-plugin");
    v.backend_mask = MLN_PLUGIN_BACKEND_OPENGL | MLN_PLUGIN_BACKEND_METAL;
    v.properties = properties;
    v.property_count = std::size(properties);
    v.geometry_type_mask = MLN_PLUGIN_GEOMETRY_POLYGON;
    v.shaders = shaders;
    v.shader_count = std::size(shaders);
    v.create_layout = createLayout;
    v.layout_feature = layoutFeature;
    v.finish_layout = finishLayout;
    v.destroy_layout = destroyLayout;
    v.query_feature = nullptr;
    v.update_uniform_block = updateUniformBlock;
    v.get_query_radius = nullptr;
    // Both the wall/roof pass and the shadow pass legitimately self-overlap (concave corners,
    // translucent double-blending); dedup via is3D + shared stencil ref instead of double-blending.
    v.enable_stencil_overlap_dedup = 1;
    // Ground-hugging/pitched geometry, same as fill-extrusion's own tile matrix.
    v.enable_near_clipped_matrix = 1;
    v.layout_properties = layoutProperties;
    v.layout_property_count = std::size(layoutProperties);
    return v;
}();

const mln_plugin_descriptor_v1 descriptor = {
    sizeof(mln_plugin_descriptor_v1),
    MLN_PLUGIN_ABI_VERSION_1,
    str("org.maplibre.fill-extrusion-plugin"),
    str(MLN_FILL_EXTRUSION_PLUGIN_VERSION),
    MLN_PLUGIN_ABI_VERSION_1,
    MLN_PLUGIN_ABI_VERSION_1,
    &layerType,
    1,
};

} // namespace

extern "C" mln_plugin_status mln_fill_extrusion_plugin_register(mln_plugin_register_function_v1 registerPlugin,
                                                                 char* error,
                                                                 size_t capacity) {
    if (!registerPlugin) return MLN_PLUGIN_STATUS_NOT_FOUND;
    return registerPlugin(&descriptor, error, capacity);
}
