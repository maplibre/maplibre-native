#include <mbgl/renderer/buckets/fill_bucket.hpp>
#include <mbgl/programs/fill_program.hpp>
#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/renderer/layers/render_fill_layer.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/gfx/fill_generator.hpp>

namespace mbgl {

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

void FillBucket::addFeature(const GeometryTileFeature& feature,
                            const GeometryCollection& geometry,
                            const ImagePositions& patternPositions,
                            const PatternLayerMap& patternDependencies,
                            std::size_t index,
                            const CanonicalTileID& canonical) {
    // generate buffers
    gfx::generateFillAndOutineBuffers(geometry, vertices, lineSegments, lines, triangleSegments, triangles);
    
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

void FillBucket::upload([[maybe_unused]] gfx::UploadPass& uploadPass) {
#if MLN_LEGACY_RENDERER
    if (!uploaded) {
        vertexBuffer = uploadPass.createVertexBuffer(std::move(vertices));
        lineIndexBuffer = uploadPass.createIndexBuffer(std::move(lines));
        triangleIndexBuffer = triangles.empty() ? std::optional<gfx::IndexBuffer>{}
                                                : uploadPass.createIndexBuffer(std::move(triangles));
    }

    for (auto& pair : paintPropertyBinders) {
        pair.second.upload(uploadPass);
    }
#endif // MLN_LEGACY_RENDERER

    uploaded = true;
}

bool FillBucket::hasData() const {
    return !triangleSegments.empty() || !lineSegments.empty();
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
    }
}

} // namespace mbgl
