#include <mbgl/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_impl.hpp>
#include <mbgl/renderer/layers/render_fill_extrusion_layer.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/constants.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <mapbox/earcut.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <cassert>

namespace mapbox {
namespace util {
template <>
struct nth<0, mbgl::GeometryCoordinate> {
    static int64_t get(const mbgl::GeometryCoordinate& t) { return t.x; };
};

template <>
struct nth<1, mbgl::GeometryCoordinate> {
    static int64_t get(const mbgl::GeometryCoordinate& t) { return t.y; };
};
} // namespace util
} // namespace mapbox

namespace mbgl {

using namespace style;

struct GeometryTooLongException : std::exception {};

namespace {

// Rounded footprint corners (the plan-view half of fill-extrusion-edge-radius).
// Sharp building corners are replaced by short arcs, softening silhouettes and
// wall shading. The radius is configured in meters via the
// fill-extrusion-edge-radius layout property and converted to tile units per
// canonical zoom. The vertical top bevel needs shader-side height interaction
// and lands separately.
constexpr double kEarthCircumference = 40075016.686;

GeometryCoordinates roundRingCorners(const GeometryCoordinates& ring, double radiusUnits) {
    // Rings arrive closed (first == last); operate on the open form.
    std::size_t n = ring.size();
    if (n >= 2 && ring.front() == ring.back()) {
        n -= 1;
    }
    if (n < 4) {
        // Triangles keep their sharpness — rounding degenerates them.
        return ring;
    }

    GeometryCoordinates out;
    out.reserve(n * 3 + 1);
    for (std::size_t i = 0; i < n; ++i) {
        const auto& prev = ring[(i + n - 1) % n];
        const auto& curr = ring[i];
        const auto& next = ring[(i + 1) % n];

        const Point<double> a = convertPoint<double>(prev);
        const Point<double> b = convertPoint<double>(curr);
        const Point<double> c = convertPoint<double>(next);

        const Point<double> inVec = b - a;
        const Point<double> outVec = c - b;
        const double inLen = std::sqrt(inVec.x * inVec.x + inVec.y * inVec.y);
        const double outLen = std::sqrt(outVec.x * outVec.x + outVec.y * outVec.y);
        if (inLen < 1e-6 || outLen < 1e-6) {
            out.emplace_back(curr);
            continue;
        }

        // Skip near-straight corners: rounding them only adds vertices.
        const double cross = inVec.x * outVec.y - inVec.y * outVec.x;
        const double dot = inVec.x * outVec.x + inVec.y * outVec.y;
        const double turn = std::abs(std::atan2(cross, dot));
        if (turn < 0.20) {
            out.emplace_back(curr);
            continue;
        }

        // Clamp the cut so adjacent corners never overlap.
        const double cut = std::min({radiusUnits, inLen * 0.5 - 0.5, outLen * 0.5 - 0.5});
        if (cut < 1.0) {
            out.emplace_back(curr);
            continue;
        }

        const Point<double> inDir{inVec.x / inLen, inVec.y / inLen};
        const Point<double> outDir{outVec.x / outLen, outVec.y / outLen};
        const Point<double> start = b - inDir * cut;
        const Point<double> end = b + outDir * cut;
        // Quadratic bezier (control point = the original corner) sampled at
        // t = 1/3 and 2/3: enough segments to read round at building scale
        // without exploding vertex counts.
        const Point<double> mid1 = start * (4.0 / 9.0) + b * (4.0 / 9.0) + end * (1.0 / 9.0);
        const Point<double> mid2 = start * (1.0 / 9.0) + b * (4.0 / 9.0) + end * (4.0 / 9.0);

        const auto push = [&](const Point<double>& p) {
            out.emplace_back(static_cast<int16_t>(std::lround(p.x)), static_cast<int16_t>(std::lround(p.y)));
        };
        push(start);
        push(mid1);
        push(mid2);
        push(end);
    }
    if (out.size() < 3) {
        return ring;
    }
    // Restore closure to match the input convention.
    if (ring.front() == ring.back()) {
        out.emplace_back(out.front());
    }
    return out;
}

void roundPolygonCorners(GeometryCollection& polygon, const CanonicalTileID& canonical, double radiusM) {
    // Tile units per meter at this canonical zoom (equator approximation is
    // fine for a visual radius).
    const double tileMeters = kEarthCircumference / (1u << canonical.z);
    const double radiusUnits = radiusM * (static_cast<double>(util::EXTENT) / tileMeters);
    if (radiusUnits < 1.5) {
        // Below ~1.5 units the arc collapses to the original corner.
        return;
    }
    for (auto& ring : polygon) {
        ring = roundRingCorners(ring, radiusUnits);
    }
}

} // namespace

FillExtrusionBucket::FillExtrusionBucket(
    const FillExtrusionBucket::PossiblyEvaluatedLayoutProperties& layout,
    const std::map<std::string, Immutable<style::LayerProperties>>& layerPaintProperties,
    const float zoom,
    const uint32_t)
    : edgeRadiusMeters_(layout.get<style::FillExtrusionEdgeRadius>()) {
    for (const auto& pair : layerPaintProperties) {
        paintPropertyBinders.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(pair.first),
            std::forward_as_tuple(getEvaluated<FillExtrusionLayerProperties>(pair.second), zoom));
    }
}

FillExtrusionBucket::~FillExtrusionBucket() {
    sharedVertices->release();
}

void FillExtrusionBucket::addFeature(const GeometryTileFeature& feature,
                                     const GeometryCollection& geometry,
                                     const ImagePositions& patternPositions,
                                     const PatternLayerMap& patternDependencies,
                                     std::size_t index,
                                     const CanonicalTileID& canonical) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tesselation.
        limitHoles(polygon, 500);

        const double radiusM = edgeRadiusMeters_;
        if (radiusM > 0.0) {
            roundPolygonCorners(polygon, canonical, radiusM);
        }

        std::size_t totalVertices = 0;

        for (const auto& ring : polygon) {
            totalVertices += ring.size();
            if (totalVertices > std::numeric_limits<uint16_t>::max()) throw GeometryTooLongException();
        }

        if (totalVertices == 0) continue;

        std::vector<uint32_t> flatIndices;
        flatIndices.reserve(totalVertices);

        std::size_t startVertices = vertices.elements();

        if (triangleSegments.empty() || triangleSegments.back().vertexLength + (5 * (totalVertices - 1) + 1) >
                                            std::numeric_limits<uint16_t>::max()) {
            triangleSegments.emplace_back(startVertices, triangles.elements());
        }

        auto& triangleSegment = triangleSegments.back();
        assert(triangleSegment.vertexLength <= std::numeric_limits<uint16_t>::max());
        auto triangleIndex = static_cast<uint16_t>(triangleSegment.vertexLength);

        assert(triangleIndex + (5 * (totalVertices - 1) + 1) <= std::numeric_limits<uint16_t>::max());

        for (const auto& ring : polygon) {
            std::size_t nVertices = ring.size();

            if (nVertices == 0) continue;

            std::size_t edgeDistance = 0;

#if !MLN_USE_FILL_EXTRUSION_INSTANCING
            // Smoothing exists to shade rounded facades; at radius 0 the wall
            // normals must stay exactly the stock faceted `perp` values so the
            // bucket output is byte-identical to upstream.
            const bool smoothNormals = radiusM > 0.0;
            const std::size_t nEdges = nVertices > 1 ? nVertices - 1 : 0;
            // Precompute is gated on smoothNormals: at radius 0 (the default,
            // stock GL path) skip both the heap allocation and the
            // normalization loop below entirely; `perp` is instead computed
            // inline per-edge, matching upstream exactly (see the !smoothNormals
            // branch further down).
            std::vector<Point<double>> edgeNrm;
            bool ringClosed = false;
            if (smoothNormals) {
                // Crease-aware smooth wall normals. Each wall quad is otherwise
                // flat-shaded with its edge's perpendicular, so a curved facade —
                // approximated by many short edges — shows visible facet bands.
                // We instead give each footprint vertex a normal averaged from its
                // two adjacent edges when the turn between them is gentle, so the
                // fragment shader interpolates a smooth gradient along the curve;
                // sharp building corners (turn above the crease angle) keep their
                // distinct edge normals and stay crisp.
                edgeNrm.resize(nEdges);
                for (std::size_t e = 0; e < nEdges; ++e) {
                    edgeNrm[e] = util::unit(
                        util::perp(convertPoint<double>(ring[e + 1]) - convertPoint<double>(ring[e])));
                }
                ringClosed = nVertices > 2 && ring.front() == ring.back();
            }
            // cos(60°): blend turns up to 60°, leave sharper corners faceted.
            constexpr double kCreaseCos = 0.5;
            const auto blendNormal = [&](const Point<double>& base, std::size_t neighbor, bool hasNeighbor) {
                if (!hasNeighbor) return base;
                const Point<double>& o = edgeNrm[neighbor];
                if (base.x * o.x + base.y * o.y > kCreaseCos) return util::unit(base + o);
                return base;
            };
#endif

            for (std::size_t i = 0; i < nVertices; i++) {
                const auto& p1 = ring[i];

#if MLN_USE_FILL_EXTRUSION_INSTANCING
                vertices.emplace_back(layoutVertex(p1, edgeDistance, i == nVertices - 1));
                flatIndices.emplace_back(triangleIndex);
                triangleIndex++;

                if (i < nVertices - 1) {
                    const auto& p2 = ring[i + 1];

                    const auto d1 = convertPoint<double>(p1);
                    const auto d2 = convertPoint<double>(p2);

                    const size_t dist = util::dist<uint16_t>(d1, d2);
                    if (edgeDistance + dist > static_cast<size_t>(std::numeric_limits<uint16_t>::max())) {
                        edgeDistance = 0;
                    }

                    edgeDistance += dist;
                }
#else
                vertices.emplace_back(
                    FillExtrusionBucket::layoutVertex(p1, 0, 0, 1, 1, static_cast<uint16_t>(edgeDistance)));
                flatIndices.emplace_back(triangleIndex);
                triangleIndex++;

                if (i != 0) {
                    const auto& p2 = ring[i - 1];

                    const auto d1 = convertPoint<double>(p1);
                    const auto d2 = convertPoint<double>(p2);

                    // Edge e runs ring[e] -> ring[e+1]; here p2=ring[e], p1=ring[e+1].
                    const std::size_t e = i - 1;
                    // At radius 0 (!smoothNormals) edgeNrm was never populated;
                    // compute perp inline exactly as stock upstream does.
                    const Point<double> perp = smoothNormals ? edgeNrm[e] : util::unit(util::perp(d1 - d2));
                    const std::size_t prevE = (e == 0) ? nEdges - 1 : e - 1;
                    const std::size_t nextE = (e + 1 >= nEdges) ? 0 : e + 1;
                    const bool hasPrev = (e != 0) || ringClosed;
                    const bool hasNext = (e + 1 < nEdges) || ringClosed;
                    // Smoothed normal at each endpoint (averaged with the
                    // adjacent edge across gentle turns, faceted at corners).
                    // Only applies when rounding is active (radiusM > 0);
                    // at radius 0 this must stay the stock faceted `perp`.
                    const Point<double> nP1 = smoothNormals ? blendNormal(perp, nextE, hasNext)
                                                            : perp; // at ring[e+1] = p1
                    const Point<double> nP2 = smoothNormals ? blendNormal(perp, prevE, hasPrev)
                                                            : perp; // at ring[e]   = p2

                    const size_t dist = util::dist<int16_t>(d1, d2);
                    if (edgeDistance + dist > static_cast<size_t>(std::numeric_limits<int16_t>::max())) {
                        edgeDistance = 0;
                    }

                    vertices.emplace_back(
                        FillExtrusionBucket::layoutVertex(p1, nP1.x, nP1.y, 0, 0, static_cast<uint16_t>(edgeDistance)));
                    vertices.emplace_back(
                        FillExtrusionBucket::layoutVertex(p1, nP1.x, nP1.y, 0, 1, static_cast<uint16_t>(edgeDistance)));

                    edgeDistance += dist;

                    vertices.emplace_back(
                        FillExtrusionBucket::layoutVertex(p2, nP2.x, nP2.y, 0, 0, static_cast<uint16_t>(edgeDistance)));
                    vertices.emplace_back(
                        FillExtrusionBucket::layoutVertex(p2, nP2.x, nP2.y, 0, 1, static_cast<uint16_t>(edgeDistance)));

                    // ┌──────┐
                    // │ 0  1 │ Counter-Clockwise winding order.
                    // │      │ Triangle 1: 0 => 2 => 1
                    // │ 2  3 │ Triangle 2: 1 => 2 => 3
                    // └──────┘
                    triangles.emplace_back(triangleIndex, triangleIndex + 2, triangleIndex + 1);
                    triangles.emplace_back(triangleIndex + 1, triangleIndex + 2, triangleIndex + 3);
                    triangleIndex += 4;
                    triangleSegment.vertexLength += 4;
                    triangleSegment.indexLength += 6;
                }
#endif
            }
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);

        std::size_t nIndices = indices.size();
        assert(nIndices % 3 == 0);

        for (std::size_t i = 0; i < nIndices; i += 3) {
            // Counter-Clockwise winding order.
            triangles.emplace_back(static_cast<uint16_t>(flatIndices[indices[i]]),
                                   static_cast<uint16_t>(flatIndices[indices[i + 2]]),
                                   static_cast<uint16_t>(flatIndices[indices[i + 1]]));
        }

        triangleSegment.vertexLength += totalVertices;
        triangleSegment.indexLength += nIndices;
    }

    for (auto& pair : paintPropertyBinders) {
        const auto it = patternDependencies.find(pair.first);
        if (it != patternDependencies.end()) {
            pair.second.populateVertexVectors(
                feature, vertices.elements(), index, patternPositions, it->second, canonical);
        } else {
            pair.second.populateVertexVectors(feature, vertices.elements(), index, patternPositions, {}, canonical);
        }
    }
}

void FillExtrusionBucket::upload([[maybe_unused]] gfx::UploadPass& uploadPass) {
    uploaded = true;
}

bool FillExtrusionBucket::hasData() const {
    return !triangleSegments.empty();
}

float FillExtrusionBucket::getQueryRadius(const RenderLayer& layer) const {
    const auto& evaluated = getEvaluated<FillExtrusionLayerProperties>(layer.evaluatedProperties);
    const std::array<float, 2>& translate = evaluated.get<FillExtrusionTranslate>();
    return util::length(translate[0], translate[1]);
}

void FillExtrusionBucket::update(const FeatureStates& states,
                                 const GeometryTileLayer& layer,
                                 const std::string& layerID,
                                 const ImagePositions& imagePositions) {
    auto it = paintPropertyBinders.find(layerID);
    if (it != paintPropertyBinders.end()) {
        it->second.updateVertexVectors(states, layer, imagePositions);
        uploaded = false;

        sharedVertices->updateModified();
    }
}

std::array<float, 3> FillExtrusionBucket::lightColor(const EvaluatedLight& light) {
    const auto color = light.get<LightColor>();
    return {{color.r, color.g, color.b}};
}

std::array<float, 3> FillExtrusionBucket::lightPosition(const EvaluatedLight& light, const TransformState& state) {
    auto lightPos = light.get<LightPosition>().getCartesian();
    mat3 lightMat;
    matrix::identity(lightMat);
    if (light.get<LightAnchor>() == LightAnchorType::Viewport) {
        matrix::rotate(lightMat, lightMat, -state.getBearing());
    }
    matrix::transformMat3f(lightPos, lightPos, lightMat);
    return lightPos;
}

float FillExtrusionBucket::lightIntensity(const EvaluatedLight& light) {
    return light.get<LightIntensity>();
}

} // namespace mbgl
