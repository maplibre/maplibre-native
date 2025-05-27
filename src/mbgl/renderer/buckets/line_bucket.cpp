#include <mbgl/renderer/buckets/line_bucket.hpp>
#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/gfx/polyline_generator.hpp>

#include <cassert>
#include <utility>

namespace mbgl {

using namespace style;

LineBucket::LineBucket(LineBucket::PossiblyEvaluatedLayoutProperties layout_,
                       const std::map<std::string, Immutable<LayerProperties>>& layerPaintProperties,
                       const float zoom_,
                       const uint32_t overscaling_)
    : layout(std::move(layout_)),
      zoom(zoom_),
      overscaling(overscaling_) {
    for (const auto& pair : layerPaintProperties) {
        paintPropertyBinders.emplace(std::piecewise_construct,
                                     std::forward_as_tuple(pair.first),
                                     std::forward_as_tuple(getEvaluated<LineLayerProperties>(pair.second), zoom));
    }
}

LineBucket::~LineBucket() {
    sharedVertices->release();
}

void LineBucket::addFeature(const GeometryTileFeature& feature,
                            const GeometryCollection& geometryCollection,
                            const ImagePositions& patternPositions,
                            const PatternLayerMap& patternDependencies,
                            std::size_t index,
                            const CanonicalTileID& canonical) {
    for (auto& line : geometryCollection) {
        addGeometry(line, feature, canonical);
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
        while (i < len - 1 && coordinates[i] == coordinates[i + 1]) {
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

    const auto& props = feature.getProperties();
    auto clip_start_it = props.find("mapbox_clip_start");
    auto clip_end_it = props.find("mapbox_clip_end");
    if (clip_start_it != props.end() && clip_end_it != props.end()) {
        double total_length = 0.0;
        for (std::size_t i = first; i < len - 1; ++i) {
            total_length += util::dist<double>(coordinates[i], coordinates[i + 1]);
        }

        options.clipDistances = gfx::PolylineGeneratorDistances{
            *numericValue<double>(clip_start_it->second), *numericValue<double>(clip_end_it->second), total_length};
    }

    options.joinType = layout.evaluate<LineJoin>(zoom, feature, canonical);
    options.miterLimit = options.joinType == LineJoinType::Bevel ? 1.05f
                                                                 : static_cast<float>(layout.get<LineMiterLimit>());
    options.beginCap = layout.get<LineCap>();
    options.endCap = options.type == FeatureType::Polygon ? LineCapType::Butt : LineCapType(layout.get<LineCap>());
    options.roundLimit = layout.get<LineRoundLimit>();
    options.overscaling = overscaling;

    generator.generate(coordinates, options);
}

void LineBucket::upload([[maybe_unused]] gfx::UploadPass& uploadPass) {
    uploaded = true;
}

bool LineBucket::hasData() const {
    return !segments.empty();
}

template <class Property>
static float get(const LinePaintProperties::PossiblyEvaluated& evaluated,
                 const std::string& id,
                 const std::map<std::string, LineBinders>& paintPropertyBinders) {
    auto it = paintPropertyBinders.find(id);
    if (it == paintPropertyBinders.end() || !it->second.statistics<Property>().max()) {
        return evaluated.get<Property>().constantOr(Property::defaultValue());
    } else {
        return *it->second.statistics<Property>().max();
    }
}

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

} // namespace mbgl
