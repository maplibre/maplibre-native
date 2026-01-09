#pragma once
#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/layout/layout.hpp>
#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/util/containers.hpp>

#include <list>

namespace mbgl {

class PatternDependency {
public:
    std::string min;
    std::string mid;
    std::string max;
};

using PatternLayerMap = mbgl::unordered_map<std::string, PatternDependency>;

class PatternFeature {
public:
    PatternFeature(std::size_t i_,
                   std::unique_ptr<GeometryTileFeature>&& feature_,
                   PatternLayerMap&& patterns_,
                   float sortKey_ = 0.0f)
        : i(i_),
          feature(std::move(feature_)),
          sortKey(sortKey_),
          patterns(std::make_unique<PatternLayerMap>(std::move(patterns_))) {}
    PatternFeature(const PatternFeature&) = delete;
    PatternFeature(PatternFeature&&) = default;
    PatternFeature& operator=(const PatternFeature&) = delete;
    PatternFeature& operator=(PatternFeature&&) = default;

    const PatternLayerMap& getPatterns() const { return *patterns; }

    friend bool operator<(const PatternFeature& lhs, const PatternFeature& rhs) { return lhs.sortKey < rhs.sortKey; }

    std::size_t i;
    std::unique_ptr<GeometryTileFeature> feature;
    float sortKey;

protected:
    std::unique_ptr<PatternLayerMap> patterns;
};

template <typename SortKeyPropertyType>
struct PatternFeatureInserter;

template <>
struct PatternFeatureInserter<void> {
    template <typename PropertiesType>
    static void insert(std::vector<PatternFeature>& features,
                       std::size_t index,
                       std::unique_ptr<GeometryTileFeature>&& feature,
                       PatternLayerMap&& patternDependencyMap,
                       float /*zoom*/,
                       const PropertiesType&,
                       const CanonicalTileID&) {
        features.emplace_back(index, std::move(feature), std::move(patternDependencyMap));
    }
};

template <class SortKeyPropertyType>
struct PatternFeatureInserter {
    template <typename PropertiesType>
    static void insert(std::vector<PatternFeature>& features,
                       std::size_t index,
                       std::unique_ptr<GeometryTileFeature>&& feature,
                       PatternLayerMap&& patternDependencyMap,
                       float zoom,
                       const PropertiesType& properties,
                       const CanonicalTileID& canonical) {
        const auto& sortKeyProperty = properties.template get<SortKeyPropertyType>();
        float sortKey = sortKeyProperty.evaluate(*feature, zoom, canonical, SortKeyPropertyType::defaultValue());
        PatternFeature patternFeature{index, std::move(feature), std::move(patternDependencyMap), sortKey};
        // NOLINTNEXTLINE(modernize-use-ranges) C++26
        const auto lowerBound = std::lower_bound(features.cbegin(), features.cend(), patternFeature);
        features.insert(lowerBound, std::move(patternFeature));
    }
};

template <class BucketType,
          class LayerPropertiesType,
          class PatternPropertyType,
          class LayoutPropertiesType = typename style::Properties<>,
          class SortKeyPropertyType = void>
class PatternLayout : public Layout {
public:
    PatternLayout(const BucketParameters& parameters,
                  const std::vector<Immutable<style::LayerProperties>>& group,
                  std::unique_ptr<GeometryTileLayer>&& sourceLayer_,
                  const LayoutParameters& layoutParameters)
        : sourceLayer(std::move(sourceLayer_)),
          zoom(parameters.tileID.overscaledZ),
          overscaling(parameters.tileID.overscaleFactor()),
          hasPattern(false) {
        assert(!group.empty());
        auto leaderLayerProperties = staticImmutableCast<LayerPropertiesType>(group.front());
        layout = leaderLayerProperties->layerImpl().layout.evaluate(PropertyEvaluationParameters(zoom));
        sourceLayerID = leaderLayerProperties->layerImpl().sourceLayer;
        bucketLeaderID = leaderLayerProperties->layerImpl().id;

        for (const auto& layerProperties : group) {
            const std::string& layerId = layerProperties->baseImpl->id;
            const auto& evaluated = style::getEvaluated<LayerPropertiesType>(layerProperties);
            const auto& patternProperty = evaluated.template get<PatternPropertyType>();
            const auto constantPattern = patternProperty.constantOr(
                Faded<style::expression::Image>{.from = "", .to = ""});
            // determine if layer group has any layers that use *-pattern
            // property and add constant pattern dependencies.
            if (!patternProperty.isConstant()) {
                hasPattern = true;
            } else if (!constantPattern.to.id().empty()) {
                hasPattern = true;
                layoutParameters.imageDependencies.emplace(constantPattern.to.id(), ImageType::Pattern);
                layoutParameters.imageDependencies.emplace(constantPattern.from.id(), ImageType::Pattern);
            }
            layerPropertiesMap.emplace(layerId, layerProperties);
        }

        const size_t featureCount = sourceLayer->featureCount();
        for (size_t i = 0; i < featureCount; ++i) {
            auto feature = sourceLayer->getFeature(i);
            if (!leaderLayerProperties->layerImpl().filter(
                    style::expression::EvaluationContext(this->zoom, feature.get())
                        .withCanonicalTileID(&parameters.tileID.canonical)))
                continue;

            PatternLayerMap patternDependencyMap;
            if (hasPattern) {
                for (const auto& layerProperties : group) {
                    const std::string& layerId = layerProperties->baseImpl->id;
                    const auto it = layerPropertiesMap.find(layerId);
                    if (it != layerPropertiesMap.end()) {
                        const auto paint = static_cast<const LayerPropertiesType&>(*it->second).evaluated;
                        const auto& patternProperty = paint.template get<PatternPropertyType>();
                        if (!patternProperty.isConstant()) {
                            // For layers with non-data-constant pattern
                            // properties, evaluate their expression and add the
                            // patterns to the dependency vector
                            const auto min = patternProperty.evaluate(*feature,
                                                                      zoom - 1,
                                                                      layoutParameters.availableImages,
                                                                      parameters.tileID.canonical,
                                                                      PatternPropertyType::defaultValue());
                            const auto mid = patternProperty.evaluate(*feature,
                                                                      zoom,
                                                                      layoutParameters.availableImages,
                                                                      parameters.tileID.canonical,
                                                                      PatternPropertyType::defaultValue());
                            const auto max = patternProperty.evaluate(*feature,
                                                                      zoom + 1,
                                                                      layoutParameters.availableImages,
                                                                      parameters.tileID.canonical,
                                                                      PatternPropertyType::defaultValue());

                            layoutParameters.imageDependencies.emplace(min.to.id(), ImageType::Pattern);
                            layoutParameters.imageDependencies.emplace(mid.to.id(), ImageType::Pattern);
                            layoutParameters.imageDependencies.emplace(max.to.id(), ImageType::Pattern);
                            patternDependencyMap.emplace(layerId,
                                                         PatternDependency{min.to.id(), mid.to.id(), max.to.id()});
                        }
                    }
                }
            }

            PatternFeatureInserter<SortKeyPropertyType>::insert(features,
                                                                i,
                                                                std::move(feature),
                                                                std::move(patternDependencyMap),
                                                                zoom,
                                                                layout,
                                                                parameters.tileID.canonical);
        }
    };

    bool hasDependencies() const override { return hasPattern; }

    void createBucket(const ImagePositions& patternPositions,
                      std::unique_ptr<FeatureIndex>& featureIndex,
                      mbgl::unordered_map<std::string, LayerRenderData>& renderData,
                      const bool /*firstLoad*/,
                      const bool /*showCollisionBoxes*/,
                      const CanonicalTileID& canonical) override {
        auto bucket = std::make_shared<BucketType>(layout, layerPropertiesMap, zoom, overscaling);
        for (auto& patternFeature : features) {
            const auto i = patternFeature.i;
            std::unique_ptr<GeometryTileFeature> feature = std::move(patternFeature.feature);
            const PatternLayerMap& patterns = patternFeature.getPatterns();
            const GeometryCollection& geometries = feature->getGeometries();

            bucket->addFeature(*feature, geometries, patternPositions, patterns, i, canonical);
            featureIndex->insert(geometries, i, sourceLayerID, bucketLeaderID);
        }
        if (bucket->hasData()) {
            for (const auto& pair : layerPropertiesMap) {
                renderData.emplace(pair.first, LayerRenderData{bucket, pair.second});
            }
        }
    };

protected:
    std::map<std::string, Immutable<style::LayerProperties>> layerPropertiesMap;
    std::string bucketLeaderID;

    const std::unique_ptr<GeometryTileLayer> sourceLayer;
    std::vector<PatternFeature> features;
    typename LayoutPropertiesType::PossiblyEvaluated layout;

    const float zoom;
    const uint32_t overscaling;
    std::string sourceLayerID;
    bool hasPattern;
};

} // namespace mbgl
