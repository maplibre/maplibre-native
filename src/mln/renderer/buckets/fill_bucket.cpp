#include <mln/renderer/buckets/fill_bucket.hpp>
#include <mln/renderer/bucket_parameters.hpp>
#include <mln/style/layers/fill_layer_impl.hpp>
#include <mln/renderer/layers/render_fill_layer.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/math.hpp>
#include <mln/gfx/fill_generator.hpp>

#include <mutex>

namespace mln {

namespace {

bool shouldWarnAboutMixedSDFPatterns(const std::string& layerID) {
    static std::mutex mutex;
    static mln::unordered_set<std::string> warnedLayers;

    const std::lock_guard lock(mutex);
    return warnedLayers.emplace(layerID).second;
}

} // namespace

FillBucket::FillBucket(const FillBucket::PossiblyEvaluatedLayoutProperties&,
                       const std::map<std::string, Immutable<style::LayerProperties>>& layerPaintProperties,
                       const float zoom,
                       const uint32_t) {
    using namespace style;
    for (const auto& pair : layerPaintProperties) {
        paintPropertyBinders.emplace(std::piecewise_construct,
                                     std::forward_as_tuple(pair.first),
                                     std::forward_as_tuple(getEvaluated<FillLayerProperties>(pair.second), zoom));
    }
}

FillBucket::~FillBucket() {
    sharedVertices->release();
}

void FillBucket::recordSDFPattern(const std::string& layerID, const bool sdf) {
    const auto [it, inserted] = sdfPatterns.emplace(layerID, sdf);
    if (!inserted && it->second != sdf && shouldWarnAboutMixedSDFPatterns(layerID)) {
        Log::Warning(Event::Style,
                     "Style sheet warning: Cannot mix SDF and non-SDF fill patterns in layer \"" + layerID + "\"");
    }
}

bool FillBucket::isSDFPattern(const std::string& layerID) const {
    const auto it = sdfPatterns.find(layerID);
    return it != sdfPatterns.end() && it->second;
}

// MLN_TRIANGULATE_FILL_OUTLINES is defined in fill_bucket.hpp
#if MLN_TRIANGULATE_FILL_OUTLINES
void FillBucket::addFeature(const GeometryTileFeature& feature,
                            const GeometryCollection& geometry,
                            const ImagePositions& patternPositions,
                            const PatternLayerMap& patternDependencies,
                            std::size_t index,
                            const CanonicalTileID& canonical) {
    // generate buffers
    gfx::generateFillAndOutineBuffers(geometry,
                                      vertices,
                                      triangles,
                                      triangleSegments,
                                      lineVertices,
                                      lineIndexes,
                                      lineSegments,
                                      basicLines,
                                      basicLineSegments);

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
#else  // MLN_TRIANGULATE_FILL_OUTLINES
void FillBucket::addFeature(const GeometryTileFeature& feature,
                            const GeometryCollection& geometry,
                            const ImagePositions& patternPositions,
                            const PatternLayerMap& patternDependencies,
                            std::size_t index,
                            const CanonicalTileID& canonical) {
    // generate buffers
    gfx::generateFillAndOutineBuffers(geometry, vertices, triangles, triangleSegments, basicLines, basicLineSegments);

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
#endif // MLN_TRIANGULATE_FILL_OUTLINES

void FillBucket::upload([[maybe_unused]] gfx::UploadPass& uploadPass) {
    uploaded = true;
}

bool FillBucket::hasData() const {
    return !triangleSegments.empty() || !basicLineSegments.empty();
}

float FillBucket::getQueryRadius(const RenderLayer& layer) const {
    using namespace style;
    const auto& evaluated = getEvaluated<FillLayerProperties>(layer.evaluatedProperties);
    const std::array<float, 2>& translate = evaluated.get<FillTranslate>();
    return util::length(translate[0], translate[1]);
}

void FillBucket::update(const FeatureStates& states,
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
