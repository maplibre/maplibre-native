#include <mln/renderer/buckets/line_bucket.hpp>
#include <mln/renderer/bucket_parameters.hpp>
#include <mln/style/layers/line_layer_impl.hpp>
#include <mln/util/math.hpp>
#include <mln/util/constants.hpp>
#include <mln/gfx/polyline_generator.hpp>

#include <cassert>
#include <utility>

namespace mln {

using namespace style;

LineBucket::LineBucket(LineBucket::PossiblyEvaluatedLayoutProperties layout_,
                       const std::map<std::string, Immutable<LayerProperties>>& layerPaintProperties,
                       const float zoom_,
                       const uint32_t overscaling_)
    : layout(std::move(layout_)),
      zoom(zoom_),
      overscaling(overscaling_) {
    for (const auto& pair : layerPaintProperties) {
        const auto& lineProps = static_cast<const LineLayerProperties&>(*pair.second);
#if MLN_USE_LINE_INTERSECTION_ROUNDED_CORNER
        const auto& intersectionRoundedCornerProperty =
            lineProps.layerImpl().paint.get<LineIntersectionRoundedCornerDistance>().value;
        intersectionRoundedCornerEnabled |= !intersectionRoundedCornerProperty.isUndefined();
#endif
        paintPropertyBinders.emplace(std::piecewise_construct,
                                     std::forward_as_tuple(pair.first),
                                     std::forward_as_tuple(lineProps.evaluated, zoom));
    }
}

LineBucket::~LineBucket() {
    sharedVertices->release();
    sharedTriangleInstances->release();
}

void LineBucket::addFeature(const GeometryTileFeature& feature,
                            const GeometryCollection& geometryCollection,
                            const ImagePositions& patternPositions,
                            const PatternLayerMap& patternDependencies,
                            std::size_t index,
                            const CanonicalTileID& canonical) {
    const auto vertexOffset = vertices.elements();

    for (auto& line : geometryCollection) {
        addGeometry(line, feature, canonical);

        if (intersectionRoundedCornerEnabled) {
            for (size_t i = 1; i < line.size(); i++) {
                auto& point = line[i];
                auto& prev = line[i - 1];
                if (point == prev) {
                    continue;
                }
                neighborsMap[point].emplace_back(std::make_pair(prev, vertexOffset * 2.0 + 0));
                neighborsMap[prev].emplace_back(std::make_pair(point, vertexOffset * 2.0 + 1));
            }
        }
    }

    if (intersectionRoundedCornerEnabled) {
        triangleInstances.clear();
        intersectionTriangles.clear();
        intersectionTriangles.emplace_back(0, 1, 2);
        intersectionSegments.clear();
        intersectionSegments.emplace_back(0, 0, 3, 3, 0, 0);
        for (auto& pair : neighborsMap) {
            auto& neighbors = pair.second;
            if (neighbors.size() <= 2) {
                continue;
            }
            auto& cornerPoint = pair.first;
            std::sort(
                neighbors.begin(), neighbors.end(), [cornerPoint](const CoordIndexPair& a, const CoordIndexPair& b) {
                    return util::angle_to(cornerPoint, a.first) > util::angle_to(cornerPoint, b.first);
                });
            auto minInterator = std::min_element(
                neighbors.begin(), neighbors.end(), [cornerPoint](const CoordIndexPair& a, const CoordIndexPair& b) {
                    return util::distSqr<float>(cornerPoint, a.first) > util::distSqr<float>(cornerPoint, b.first);
                });
            [[maybe_unused]] auto maxCornerDistance = static_cast<uint32_t>(
                util::dist<double>(cornerPoint, minInterator->first) / 2.0);
            for (size_t i = 0; i < neighbors.size(); i++) {
                auto nextIndex = (i + 1) % neighbors.size();
                auto& prevPoint = neighbors[i].first;
                auto& nextPoint = neighbors[nextIndex].first;

                auto angle = std::abs(
                    util::rad2deg(util::angle_between(prevPoint - cornerPoint, nextPoint - cornerPoint)));
                if (angle > 170.0 && angle < 190.0) {
                    continue;
                }
                auto& prevDataIndex = neighbors[i].second;
                auto& nextDataIndex = neighbors[nextIndex].second;

                triangleInstances.emplace_back(
                    triangleInstance(prevPoint, cornerPoint, nextPoint, prevDataIndex, nextDataIndex));
                intersectionSegments.back().instanceCount++;
            }
        }
    }

    if (retainFeaturesById) {
        if (const auto vertexCount = vertices.elements() - vertexOffset; vertexCount > 0) {
            retainFeature(feature, vertexOffset, vertexCount);
        }
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

void LineBucket::addGeometry(const GeometryCoordinates& coordinates,
                             const GeometryTileFeature& feature,
                             const CanonicalTileID& canonical) {
    // Ignore empty coordinates.
    if (coordinates.empty()) {
        return;
    }
    gfx::PolylineGenerator<LineLayoutVertex, SegmentBase> generator(
        vertices,
        LineBucket::layoutVertex,
        segments,
        [](std::size_t vertexOffset, std::size_t indexOffset) -> SegmentBase {
            return SegmentBase(vertexOffset, indexOffset);
        },
        [](auto& seg) -> SegmentBase& { return seg; },
        triangles);

    gfx::PolylineGeneratorOptions options;

    options.type = feature.getType();
    const std::size_t len = [&coordinates] {
        std::size_t l = coordinates.size();
        // If the line has duplicate vertices at the end, adjust length to remove them.
        while (l >= 2 && coordinates[l - 1] == coordinates[l - 2]) {
            l--;
        }
        return l;
    }();

    const std::size_t first = [&coordinates, &len] {
        std::size_t i = 0;
        // If the line has duplicate vertices at the start, adjust index to remove them.
        while (i + 1 < len && coordinates[i] == coordinates[i + 1]) {
            i++;
        }
        return i;
    }();

    // Ignore invalid geometry.
    const std::size_t minLen = (options.type == FeatureType::Polygon ? 3 : 2);
    if (len < minLen) {
        // Warn once, but only if the source geometry is invalid, not if de-duplication made it invalid.
        // This happens, e.g., when attempting to use a GeoJSON `MultiPoint`
        // or `MLNPointCollectionFeature` as the source for a line layer.
        // Unfortunately, we cannot show the layer or source name from here.
        if (coordinates.size() < minLen) {
            static bool warned = false; // not thread-safe, there's a small chance of warning more than once
            if (!warned) {
                warned = true;
                Log::Warning(Event::General, "Invalid geometry in line layer");
            }
        }
        return;
    }

    const auto clip_start = feature.getValue("mapbox_clip_start");
    if (clip_start) {
        const auto clip_end = feature.getValue("mapbox_clip_end");
        if (clip_end) {
            const auto startValue = numericValue<double>(*clip_start);
            const auto endValue = numericValue<double>(*clip_end);
            if (startValue && endValue && *startValue < *endValue) {
                double total_length = 0.0;
                for (std::size_t i = first; i < len - 1; ++i) {
                    total_length += util::dist<double>(coordinates[i], coordinates[i + 1]);
                }

                options.clipDistances = gfx::PolylineGeneratorDistances{*startValue, *endValue, total_length};
            }
        }
    }

    options.joinType = layout.evaluate<LineJoin>(zoom, feature, canonical);
    options.miterLimit = options.joinType == LineJoinType::Bevel ? 1.05f : layout.get<LineMiterLimit>();
    options.beginCap = layout.get<LineCap>();
    options.endCap = options.type == FeatureType::Polygon ? LineCapType::Butt : layout.get<LineCap>();
    options.roundLimit = layout.get<LineRoundLimit>();
    options.overscaling = overscaling;

    generator.generate(coordinates, options);
}

void LineBucket::upload([[maybe_unused]] gfx::UploadPass& uploadPass) {
    uploaded = true;
    // Layout is final: once these are in GPU buffers the CPU copies are dropped, unless the map
    // reads them back to report rendered features.
    if (retainFeaturesById) {
        return;
    }
    sharedVertices->release();
    sharedTriangles->release();
    for (auto& pair : paintPropertyBinders) {
        pair.second.release();
    }
}

bool LineBucket::hasData() const {
    return !segments.empty();
}

namespace {
template <class Property>
float get(const LinePaintProperties::PossiblyEvaluated& evaluated,
          const std::string& id,
          const std::map<std::string, LineBinders>& paintPropertyBinders) {
    auto it = paintPropertyBinders.find(id);
    if (it == paintPropertyBinders.end() || !it->second.statistics<Property>().max()) {
        return evaluated.get<Property>().constantOr(Property::defaultValue());
    } else {
        return *it->second.statistics<Property>().max();
    }
}
} // namespace

float LineBucket::getQueryRadius(const RenderLayer& layer) const {
    const auto& evaluated = getEvaluated<LineLayerProperties>(layer.evaluatedProperties);
    const std::array<float, 2>& translate = evaluated.get<LineTranslate>();
    float offset = get<LineOffset>(evaluated, layer.getID(), paintPropertyBinders);
    float lineWidth = get<LineWidth>(evaluated, layer.getID(), paintPropertyBinders);
    float gapWidth = get<LineGapWidth>(evaluated, layer.getID(), paintPropertyBinders);
    if (gapWidth) {
        lineWidth = gapWidth + 2 * lineWidth;
    }

    return lineWidth / 2.0f + std::abs(offset) + util::length(translate[0], translate[1]);
}

void LineBucket::update(const FeatureStates& states,
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

} // namespace mln
