#include <mbgl/renderer/render_orchestrator.hpp>

#include <mbgl/annotation/annotation_manager.hpp>
#include <mbgl/layermanager/layer_manager.hpp>
#if MLN_DRAWABLE_RENDERER
#include <mbgl/renderer/change_request.hpp>
#endif
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/renderer/upload_parameters.hpp>
#include <mbgl/renderer/pattern_atlas.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/transition_parameters.hpp>
#include <mbgl/renderer/property_evaluation_parameters.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/style_diff.hpp>
#include <mbgl/renderer/query.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/geometry/line_atlas.hpp>
#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/transition_options.hpp>
#include <mbgl/text/glyph_manager.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {

using namespace style;

static RendererObserver& nullObserver() {
    static RendererObserver observer;
    return observer;
}

namespace {

class LayerRenderItem final : public RenderItem {
public:
    LayerRenderItem(RenderLayer& layer_, RenderSource* source_, uint32_t index_)
        : layer(layer_),
          source(source_),
          index(index_) {}
    bool operator<(const LayerRenderItem& other) const { return index < other.index; }

    std::reference_wrapper<RenderLayer> layer;
    RenderSource* source;
    const uint32_t index;

private:
    bool hasRenderPass(RenderPass pass) const override { return layer.get().hasRenderPass(pass); }
    void upload(gfx::UploadPass& pass) const override { layer.get().upload(pass); }
    void render(PaintParameters& parameters) const override { layer.get().render(parameters); }
    const std::string& getName() const override { return layer.get().getID(); }
};

class RenderTreeImpl final : public RenderTree {
public:
    RenderTreeImpl(std::unique_ptr<RenderTreeParameters> parameters_,
                   std::set<LayerRenderItem> layerRenderItems_,
                   std::vector<std::unique_ptr<RenderItem>> sourceRenderItems_,
                   LineAtlas& lineAtlas_,
                   PatternAtlas& patternAtlas_,
                   RenderLayerReferences layersNeedPlacement_,
                   Immutable<Placement> placement_,
                   bool updateSymbolOpacities_)
        : RenderTree(std::move(parameters_)),
          layerRenderItems(std::move(layerRenderItems_)),
          sourceRenderItems(std::move(sourceRenderItems_)),
          lineAtlas(lineAtlas_),
          patternAtlas(patternAtlas_),
          layersNeedPlacement(std::move(layersNeedPlacement_)),
          placement(std::move(placement_)),
          updateSymbolOpacities(updateSymbolOpacities_) {}

    void prepare() override {
        for (auto it = layersNeedPlacement.rbegin(); it != layersNeedPlacement.rend(); ++it) {
            placement->updateLayerBuckets(*it, parameters->transformParams.state, updateSymbolOpacities);
        }
    }

    RenderItems getLayerRenderItems() const override { return {layerRenderItems.begin(), layerRenderItems.end()}; }
    RenderItems getSourceRenderItems() const override {
        RenderItems result;
        result.reserve(sourceRenderItems.size());
        for (const auto& item : sourceRenderItems) result.emplace_back(*item);
        return result;
    }
    LineAtlas& getLineAtlas() const override { return lineAtlas; }
    PatternAtlas& getPatternAtlas() const override { return patternAtlas; }

    std::set<LayerRenderItem> layerRenderItems;
    std::vector<std::unique_ptr<RenderItem>> sourceRenderItems;
    std::reference_wrapper<LineAtlas> lineAtlas;
    std::reference_wrapper<PatternAtlas> patternAtlas;
    RenderLayerReferences layersNeedPlacement;
    Immutable<Placement> placement;
    bool updateSymbolOpacities;
};

} // namespace

RenderOrchestrator::RenderOrchestrator(bool backgroundLayerAsColor_, const std::optional<std::string>& localFontFamily_)
    : observer(&nullObserver()),
      glyphManager(std::make_unique<GlyphManager>(std::make_unique<LocalGlyphRasterizer>(localFontFamily_))),
      imageManager(std::make_unique<ImageManager>()),
      lineAtlas(std::make_unique<LineAtlas>()),
      patternAtlas(std::make_unique<PatternAtlas>()),
      imageImpls(makeMutable<std::vector<Immutable<style::Image::Impl>>>()),
      sourceImpls(makeMutable<std::vector<Immutable<style::Source::Impl>>>()),
      layerImpls(makeMutable<std::vector<Immutable<style::Layer::Impl>>>()),
      renderLight(makeMutable<Light::Impl>()),
      backgroundLayerAsColor(backgroundLayerAsColor_) {
    glyphManager->setObserver(this);
    imageManager->setObserver(this);
}

RenderOrchestrator::~RenderOrchestrator() {
    if (contextLost) {
        // Signal all RenderLayers that the context was lost
        // before cleaning up. At the moment, only CustomLayer is
        // interested whether rendering context is lost. However, it would be
        // beneficial for dynamically loaded or other custom built-in plugins.
        for (const auto& entry : renderLayers) {
            RenderLayer& layer = *entry.second;
            layer.markContextDestroyed();
        }
    }
};

void RenderOrchestrator::setObserver(RendererObserver* observer_) {
    observer = observer_ ? observer_ : &nullObserver();
}

std::unique_ptr<RenderTree> RenderOrchestrator::createRenderTree(
    const std::shared_ptr<UpdateParameters>& updateParameters) {
    const bool isMapModeContinuous = updateParameters->mode == MapMode::Continuous;
    if (!isMapModeContinuous) {
        // Reset zoom history state.
        zoomHistory.first = true;
    }

    if (LayerManager::annotationsEnabled) {
        auto guard = updateParameters->annotationManager.lock();
        if (updateParameters->annotationManager) {
            updateParameters->annotationManager->updateData();
        }
    }

    const bool zoomChanged = zoomHistory.update(static_cast<float>(updateParameters->transformState.getZoom()),
                                                updateParameters->timePoint);

    const TransitionOptions transitionOptions = isMapModeContinuous ? updateParameters->transitionOptions
                                                                    : TransitionOptions();

    const TransitionParameters transitionParameters{updateParameters->timePoint, transitionOptions};

    const PropertyEvaluationParameters evaluationParameters{
        zoomHistory,
        updateParameters->timePoint,
        transitionOptions.duration.value_or(isMapModeContinuous ? util::DEFAULT_TRANSITION_DURATION
                                                                : Duration::zero())};

    const TileParameters tileParameters{updateParameters->pixelRatio,
                                        updateParameters->debugOptions,
                                        updateParameters->transformState,
                                        updateParameters->fileSource,
                                        updateParameters->mode,
                                        updateParameters->annotationManager,
                                        *imageManager,
                                        *glyphManager,
                                        updateParameters->prefetchZoomDelta};

    glyphManager->setURL(updateParameters->glyphURL);

    // Update light.
    const bool lightChanged = renderLight.impl != updateParameters->light;

    if (lightChanged) {
        renderLight.impl = updateParameters->light;
        renderLight.transition(transitionParameters);
    }

    if (lightChanged || zoomChanged || renderLight.hasTransition()) {
        renderLight.evaluate(evaluationParameters);
    }

    const ImageDifference imageDiff = diffImages(imageImpls, updateParameters->images);
    imageImpls = updateParameters->images;

    // Only trigger tile reparse for changed images. Changed images only need a
    // relayout when they have a different size.
    bool hasImageDiff = !imageDiff.removed.empty();

    // Remove removed images from sprite atlas.
    for (const auto& entry : imageDiff.removed) {
        imageManager->removeImage(entry.first);
        patternAtlas->removePattern(entry.first);
    }

    // Add added images to sprite atlas.
    for (const auto& entry : imageDiff.added) {
        imageManager->addImage(entry.second);
    }

    // Update changed images.
    for (const auto& entry : imageDiff.changed) {
        if (imageManager->updateImage(entry.second.after)) {
            patternAtlas->removePattern(entry.first);
            hasImageDiff = true;
        }
    }

    imageManager->notifyIfMissingImageAdded();
    imageManager->setLoaded(updateParameters->spriteLoaded);

    const LayerDifference layerDiff = diffLayers(layerImpls, updateParameters->layers);
    layerImpls = updateParameters->layers;
    const bool layersAddedOrRemoved = !layerDiff.added.empty() || !layerDiff.removed.empty();

#if MLN_DRAWABLE_RENDERER
    std::vector<std::unique_ptr<ChangeRequest>> changes;
#endif

    // Remove render layers for removed layers.
    for (const auto& entry : layerDiff.removed) {
        const auto hit = renderLayers.find(entry.first);
        if (hit != renderLayers.end()) {
#if MLN_DRAWABLE_RENDERER
            hit->second->layerRemoved(changes);
#endif
            renderLayers.erase(hit);
        }
    }

    // Create render layers for newly added layers.
    for (const auto& entry : layerDiff.added) {
        auto renderLayer = LayerManager::get()->createRenderLayer(entry.second);
        renderLayer->transition(transitionParameters);
        renderLayers.emplace(entry.first, std::move(renderLayer));
    }

    // Update render layers for changed layers.
    for (const auto& entry : layerDiff.changed) {
        if (const auto& renderLayer = renderLayers.at(entry.first)) {
            const auto& newLayer = entry.second.after;

#if MLN_DRAWABLE_RENDERER
            renderLayer->layerChanged(transitionParameters, newLayer, changes);
#endif // MLN_DRAWABLE_RENDERER

            renderLayer->transition(transitionParameters, newLayer);
        }
    }

    if (layersAddedOrRemoved) {
        orderedLayers.clear();
        orderedLayers.reserve(layerImpls->size());
        [[maybe_unused]] int32_t layerIndex = 0;
        for (const auto& layerImpl : *layerImpls) {
            RenderLayer* layer = renderLayers.at(layerImpl->id).get();
            assert(layer);
            orderedLayers.emplace_back(*layer);

#if MLN_DRAWABLE_RENDERER
            // We're mutating the list of ordered layers and must notify them of their new assigned indices
            layer->layerIndexChanged(layerIndex++, changes);
#endif
        }
    }
    assert(orderedLayers.size() == renderLayers.size());

    if (layersAddedOrRemoved || !layerDiff.changed.empty()) {
        glyphManager->evict(fontStacks(*layerImpls));
    }

    // Update layers for class and zoom changes.
    std::unordered_set<std::string> constantsMaskChanged;
    for (RenderLayer& layer : orderedLayers) {
        const std::string& id = layer.getID();
        const bool layerAddedOrChanged = layerDiff.added.count(id) || layerDiff.changed.count(id);
        if (layerAddedOrChanged || zoomChanged || layer.hasTransition() || layer.hasCrossfade()) {
            auto previousMask = layer.evaluatedProperties->constantsMask();
            layer.evaluate(evaluationParameters);
            if (previousMask != layer.evaluatedProperties->constantsMask()) {
                constantsMaskChanged.insert(id);
            }
        }
    }

    const SourceDifference sourceDiff = diffSources(sourceImpls, updateParameters->sources);
    sourceImpls = updateParameters->sources;

    // Remove render layers for removed sources.
    for (const auto& entry : sourceDiff.removed) {
        renderSources.erase(entry.first);
    }

    // Create render sources for newly added sources.
    for (const auto& entry : sourceDiff.added) {
        std::unique_ptr<RenderSource> renderSource = RenderSource::create(entry.second);
        renderSource->setObserver(this);
        renderSources.emplace(entry.first, std::move(renderSource));
    }
    transformState = updateParameters->transformState;
    const bool tiltedView = transformState.getPitch() != 0.0f;

    // Create parameters for the render tree.
    auto renderTreeParameters = std::make_unique<RenderTreeParameters>(updateParameters->transformState,
                                                                       updateParameters->mode,
                                                                       updateParameters->debugOptions,
                                                                       updateParameters->timePoint,
                                                                       renderLight.getEvaluated());

    std::set<LayerRenderItem> layerRenderItems;
    layersNeedPlacement.clear();
    auto renderItemsEmplaceHint = layerRenderItems.begin();

    // Reserve size for filteredLayersForSource if there are sources.
    if (!sourceImpls->empty()) {
        filteredLayersForSource.reserve(layerImpls->size());
    }

    // Update all sources and initialize renderItems.
    for (const auto& sourceImpl : *sourceImpls) {
        RenderSource* source = renderSources.at(sourceImpl->id).get();
        bool sourceNeedsRendering = false;
        bool sourceNeedsRelayout = false;

        for (std::size_t index = 0; index < orderedLayers.size(); ++index) {
            RenderLayer& layer = orderedLayers[index];
            const auto* layerInfo = layer.baseImpl->getTypeInfo();
            const bool layerIsVisible = layer.baseImpl->visibility != style::VisibilityType::None;
            const bool zoomFitsLayer = layer.supportsZoom(zoomHistory.lastZoom);
            renderTreeParameters->has3D |= (layerInfo->pass3d == LayerTypeInfo::Pass3D::Required);

            if (layerInfo->source != LayerTypeInfo::Source::NotRequired) {
                if (layer.baseImpl->source == sourceImpl->id) {
                    const std::string& layerId = layer.getID();
                    sourceNeedsRelayout = (sourceNeedsRelayout || hasImageDiff || constantsMaskChanged.count(layerId) ||
                                           hasLayoutDifference(layerDiff, layerId));
                    if (layerIsVisible) {
                        filteredLayersForSource.push_back(layer.evaluatedProperties);
                        if (zoomFitsLayer) {
                            sourceNeedsRendering = true;
                            renderItemsEmplaceHint = layerRenderItems.emplace_hint(
                                renderItemsEmplaceHint, layer, source, static_cast<uint32_t>(index));
                        }
                    }
                }
                continue;
            }

            // Handle layers without source.
            if (layerIsVisible && zoomFitsLayer && sourceImpl.get() == sourceImpls->at(0).get()) {
                if (backgroundLayerAsColor && layer.baseImpl == layerImpls->front()) {
                    const auto& solidBackground = layer.getSolidBackground();
                    if (solidBackground) {
                        renderTreeParameters->backgroundColor = *solidBackground;
                        continue; // This layer is shown with background color,
                                  // and it shall not be added to render items.
                    }
                }
                renderItemsEmplaceHint = layerRenderItems.emplace_hint(
                    renderItemsEmplaceHint, layer, nullptr, static_cast<uint32_t>(index));
            }
        }
        source->update(sourceImpl, filteredLayersForSource, sourceNeedsRendering, sourceNeedsRelayout, tileParameters);
        filteredLayersForSource.clear();
    }

#if MLN_DRAWABLE_RENDERER
    // Mark layers included in the renderable set as renderable
    // @TODO: Optimize this logic, combine with the above
    for (size_t i = 0; i < orderedLayers.size(); ++i) {
        RenderLayer& layer = orderedLayers[i];
        layer.markLayerRenderable(
            layerRenderItems.find(LayerRenderItem(layer, nullptr, static_cast<uint32_t>(i))) != layerRenderItems.end(),
            changes);
    }

    // Add all change requests up to this point
    addChanges(changes);
#endif

    renderTreeParameters->loaded = updateParameters->styleLoaded && isLoaded();
    if (!isMapModeContinuous && !renderTreeParameters->loaded) {
        return nullptr;
    }

    // Prepare. Update all matrices and generate data that we should upload to the GPU.
    for (const auto& entry : renderSources) {
        if (entry.second->isEnabled()) {
            entry.second->prepare(
                {renderTreeParameters->transformParams, updateParameters->debugOptions, *imageManager});
        }
    }

    auto opaquePassCutOffEstimation = layerRenderItems.size();
    for (auto& renderItem : layerRenderItems) {
        RenderLayer& renderLayer = renderItem.layer;
        renderLayer.prepare(
            {renderItem.source, *imageManager, *patternAtlas, *lineAtlas, updateParameters->transformState});
        if (renderLayer.needsPlacement()) {
            layersNeedPlacement.emplace_back(renderLayer);
        }
        if (renderTreeParameters->opaquePassCutOff == 0) {
            --opaquePassCutOffEstimation;
            if (renderLayer.is3D()) {
                renderTreeParameters->opaquePassCutOff = uint32_t(opaquePassCutOffEstimation);
            }
        }
    }
    // Symbol placement.
    assert((updateParameters->mode == MapMode::Tile) || !placedSymbolDataCollected);
    bool symbolBucketsChanged = false;
    bool symbolBucketsAdded = false;
    std::set<std::string> usedSymbolLayers;
    const auto longitude = static_cast<float>(updateParameters->transformState.getLatLng().longitude());
    for (auto it = layersNeedPlacement.crbegin(); it != layersNeedPlacement.crend(); ++it) {
        RenderLayer& layer = *it;
        auto result = crossTileSymbolIndex.addLayer(layer, longitude);
        if (isMapModeContinuous) {
            usedSymbolLayers.insert(layer.getID());
            symbolBucketsAdded = symbolBucketsAdded || (result & CrossTileSymbolIndex::AddLayerResult::BucketsAdded);
            symbolBucketsChanged = symbolBucketsChanged || (result != CrossTileSymbolIndex::AddLayerResult::NoChanges);
        }
    }

    if (isMapModeContinuous) {
        std::optional<Duration> placementUpdatePeriodOverride;
        if (symbolBucketsAdded && !tiltedView) {
            // If the view is not tilted, we want *the new* symbols to show up
            // faster, however simple setting `placementChanged` to `true` would
            // initiate placement too often as new buckets usually come from
            // several rendered tiles in a row within a short period of time.
            // Instead, we squeeze placement update period to coalesce buckets
            // updates from several tiles. On contrary, with the tilted view
            // it's more important to make placement rarely for performance
            // reasons and as the new symbols are normally "far away" and the
            // user is not that interested to see them ASAP.
            placementUpdatePeriodOverride = std::optional<Duration>(Milliseconds(30));
        }

        renderTreeParameters->placementChanged = !placementController.placementIsRecent(
            updateParameters->timePoint,
            static_cast<float>(updateParameters->transformState.getZoom()),
            placementUpdatePeriodOverride);
        symbolBucketsChanged |= renderTreeParameters->placementChanged;
        if (renderTreeParameters->placementChanged) {
            Mutable<Placement> placement = Placement::create(updateParameters, placementController.getPlacement());
            placement->placeLayers(layersNeedPlacement);
            placementController.setPlacement(std::move(placement));
            crossTileSymbolIndex.pruneUnusedLayers(usedSymbolLayers);
            for (const auto& entry : renderSources) {
                entry.second->updateFadingTiles();
            }
        } else {
            placementController.setPlacementStale();
        }
        renderTreeParameters->symbolFadeChange = placementController.getPlacement()->symbolFadeChange(
            updateParameters->timePoint);
        renderTreeParameters->needsRepaint = hasTransitions(updateParameters->timePoint);
    } else {
        renderTreeParameters->placementChanged = symbolBucketsChanged = !layersNeedPlacement.empty();
        if (renderTreeParameters->placementChanged) {
            Mutable<Placement> placement = Placement::create(updateParameters);
            placement->collectPlacedSymbolData(placedSymbolDataCollected);
            placement->placeLayers(layersNeedPlacement);
            placementController.setPlacement(std::move(placement));
        }
        crossTileSymbolIndex.reset();
        renderTreeParameters->symbolFadeChange = 1.0f;
        renderTreeParameters->needsRepaint = false;
    }

    if (!renderTreeParameters->needsRepaint && renderTreeParameters->loaded) {
        // Notify observer about unused images when map is fully loaded
        // and there are no ongoing transitions.
        imageManager->reduceMemoryUseIfCacheSizeExceedsLimit();
    }

    std::vector<std::unique_ptr<RenderItem>> sourceRenderItems;
    for (const auto& entry : renderSources) {
        if (entry.second->isEnabled()) {
            sourceRenderItems.emplace_back(entry.second->createRenderItem());
        }
    }

    return std::make_unique<RenderTreeImpl>(std::move(renderTreeParameters),
                                            std::move(layerRenderItems),
                                            std::move(sourceRenderItems),
                                            *lineAtlas,
                                            *patternAtlas,
                                            std::move(layersNeedPlacement),
                                            placementController.getPlacement(),
                                            symbolBucketsChanged);
}

std::vector<Feature> RenderOrchestrator::queryRenderedFeatures(const ScreenLineString& geometry,
                                                               const RenderedQueryOptions& options) const {
    std::unordered_map<std::string, const RenderLayer*> layers;
    if (options.layerIDs) {
        for (const auto& layerID : *options.layerIDs) {
            if (const RenderLayer* layer = getRenderLayer(layerID)) {
                layers.emplace(layer->getID(), layer);
            }
        }
    } else {
        for (const auto& entry : renderLayers) {
            layers.emplace(entry.second->getID(), entry.second.get());
        }
    }

    return queryRenderedFeatures(geometry, options, layers);
}

void RenderOrchestrator::queryRenderedSymbols(std::unordered_map<std::string, std::vector<Feature>>& resultsByLayer,
                                              const ScreenLineString& geometry,
                                              const std::unordered_map<std::string, const RenderLayer*>& layers,
                                              const RenderedQueryOptions& options) const {
    const auto hasCrossTileIndex = [](const auto& pair) {
        return pair.second->baseImpl->getTypeInfo()->crossTileIndex == style::LayerTypeInfo::CrossTileIndex::Required;
    };

    std::unordered_map<std::string, const RenderLayer*> crossTileSymbolIndexLayers;
    std::copy_if(layers.begin(),
                 layers.end(),
                 std::inserter(crossTileSymbolIndexLayers, crossTileSymbolIndexLayers.begin()),
                 hasCrossTileIndex);

    if (crossTileSymbolIndexLayers.empty()) {
        return;
    }
    const Placement& placement = *placementController.getPlacement();
    auto renderedSymbols = placement.getCollisionIndex().queryRenderedSymbols(geometry);
    std::vector<std::reference_wrapper<const RetainedQueryData>> bucketQueryData;
    bucketQueryData.reserve(renderedSymbols.size());
    for (const auto& entry : renderedSymbols) {
        bucketQueryData.emplace_back(placement.getQueryData(entry.first));
    }
    // Although symbol query is global, symbol results are only sortable within
    // a bucket For a predictable global sort renderItems, we sort the buckets
    // based on their corresponding tile position
    std::sort(
        bucketQueryData.begin(), bucketQueryData.end(), [](const RetainedQueryData& a, const RetainedQueryData& b) {
            return std::tie(a.tileID.canonical.z, a.tileID.canonical.y, a.tileID.wrap, a.tileID.canonical.x) <
                   std::tie(b.tileID.canonical.z, b.tileID.canonical.y, b.tileID.wrap, b.tileID.canonical.x);
        });

    for (auto wrappedQueryData : bucketQueryData) {
        auto& queryData = wrappedQueryData.get();
        auto bucketSymbols = queryData.featureIndex->lookupSymbolFeatures(renderedSymbols[queryData.bucketInstanceId],
                                                                          options,
                                                                          crossTileSymbolIndexLayers,
                                                                          queryData.tileID,
                                                                          queryData.featureSortOrder);

        for (auto layer : bucketSymbols) {
            auto& resultFeatures = resultsByLayer[layer.first];
            std::move(layer.second.begin(), layer.second.end(), std::inserter(resultFeatures, resultFeatures.end()));
        }
    }
}

std::vector<Feature> RenderOrchestrator::queryRenderedFeatures(
    const ScreenLineString& geometry,
    const RenderedQueryOptions& options,
    const std::unordered_map<std::string, const RenderLayer*>& layers) const {
    std::unordered_set<std::string> sourceIDs;
    std::unordered_map<std::string, const RenderLayer*> filteredLayers;
    for (const auto& pair : layers) {
        if (!pair.second->needsRendering() || !pair.second->supportsZoom(zoomHistory.lastZoom)) {
            continue;
        }
        filteredLayers.emplace(pair);
        sourceIDs.emplace(pair.second->baseImpl->source);
    }

    mat4 projMatrix;
    transformState.getProjMatrix(projMatrix);

    std::unordered_map<std::string, std::vector<Feature>> resultsByLayer;
    for (const auto& sourceID : sourceIDs) {
        if (RenderSource* renderSource = getRenderSource(sourceID)) {
            auto sourceResults = renderSource->queryRenderedFeatures(
                geometry, transformState, filteredLayers, options, projMatrix);
            std::move(
                sourceResults.begin(), sourceResults.end(), std::inserter(resultsByLayer, resultsByLayer.begin()));
        }
    }

    queryRenderedSymbols(resultsByLayer, geometry, filteredLayers, options);

    mbgl::DynamicFeatureIndex dynamicIndex;
    for (const auto& pair : filteredLayers) {
        const RenderLayer* layer = pair.second;
        layer->populateDynamicRenderFeatureIndex(dynamicIndex);
    }
    dynamicIndex.query(resultsByLayer, geometry, transformState);
    std::vector<Feature> result;

    if (resultsByLayer.empty()) {
        return result;
    }

    // Combine all results based on the style layer renderItems.
    for (const auto& pair : filteredLayers) {
        auto it = resultsByLayer.find(pair.second->baseImpl->id);
        if (it != resultsByLayer.end()) {
            std::move(it->second.begin(), it->second.end(), std::back_inserter(result));
        }
    }

    return result;
}

std::vector<Feature> RenderOrchestrator::queryShapeAnnotations(const ScreenLineString& geometry) const {
    assert(LayerManager::annotationsEnabled);
    std::unordered_map<std::string, const RenderLayer*> shapeAnnotationLayers;
    RenderedQueryOptions options;
    for (const auto& layerImpl : *layerImpls) {
        if (std::mismatch(layerImpl->id.begin(),
                          layerImpl->id.end(),
                          AnnotationManager::ShapeLayerID.begin(),
                          AnnotationManager::ShapeLayerID.end())
                .second == AnnotationManager::ShapeLayerID.end()) {
            if (const RenderLayer* layer = getRenderLayer(layerImpl->id)) {
                shapeAnnotationLayers.emplace(layer->getID(), layer);
            }
        }
    }

    return queryRenderedFeatures(geometry, options, shapeAnnotationLayers);
}

std::vector<Feature> RenderOrchestrator::querySourceFeatures(const std::string& sourceID,
                                                             const SourceQueryOptions& options) const {
    const RenderSource* source = getRenderSource(sourceID);
    if (!source) return {};

    return source->querySourceFeatures(options);
}

FeatureExtensionValue RenderOrchestrator::queryFeatureExtensions(
    const std::string& sourceID,
    const Feature& feature,
    const std::string& extension,
    const std::string& extensionField,
    const std::optional<std::map<std::string, Value>>& args) const {
    if (RenderSource* renderSource = getRenderSource(sourceID)) {
        return renderSource->queryFeatureExtensions(feature, extension, extensionField, args);
    }
    return {};
}

void RenderOrchestrator::setFeatureState(const std::string& sourceID,
                                         const std::optional<std::string>& sourceLayerID,
                                         const std::string& featureID,
                                         const FeatureState& state) {
    if (RenderSource* renderSource = getRenderSource(sourceID)) {
        renderSource->setFeatureState(sourceLayerID, featureID, state);
    }
}

void RenderOrchestrator::getFeatureState(FeatureState& state,
                                         const std::string& sourceID,
                                         const std::optional<std::string>& sourceLayerID,
                                         const std::string& featureID) const {
    if (RenderSource* renderSource = getRenderSource(sourceID)) {
        renderSource->getFeatureState(state, sourceLayerID, featureID);
    }
}

void RenderOrchestrator::removeFeatureState(const std::string& sourceID,
                                            const std::optional<std::string>& sourceLayerID,
                                            const std::optional<std::string>& featureID,
                                            const std::optional<std::string>& stateKey) {
    if (RenderSource* renderSource = getRenderSource(sourceID)) {
        renderSource->removeFeatureState(sourceLayerID, featureID, stateKey);
    }
}

void RenderOrchestrator::reduceMemoryUse() {
    filteredLayersForSource.shrink_to_fit();
    for (const auto& entry : renderSources) {
        entry.second->reduceMemoryUse();
    }
    imageManager->reduceMemoryUse();
    observer->onInvalidate();
}

void RenderOrchestrator::dumpDebugLogs() {
    for (const auto& entry : renderSources) {
        entry.second->dumpDebugLogs();
    }

    imageManager->dumpDebugLogs();
}

void RenderOrchestrator::collectPlacedSymbolData(bool enable) {
    placedSymbolDataCollected = enable;
}

const std::vector<PlacedSymbolData>& RenderOrchestrator::getPlacedSymbolsData() const {
    return placementController.getPlacement()->getPlacedSymbolsData();
}

RenderLayer* RenderOrchestrator::getRenderLayer(const std::string& id) {
    auto it = renderLayers.find(id);
    return it != renderLayers.end() ? it->second.get() : nullptr;
}

const RenderLayer* RenderOrchestrator::getRenderLayer(const std::string& id) const {
    auto it = renderLayers.find(id);
    return it != renderLayers.end() ? it->second.get() : nullptr;
}

RenderSource* RenderOrchestrator::getRenderSource(const std::string& id) const {
    auto it = renderSources.find(id);
    return it != renderSources.end() ? it->second.get() : nullptr;
}

bool RenderOrchestrator::hasTransitions(TimePoint timePoint) const {
    if (renderLight.hasTransition()) {
        return true;
    }

    for (const auto& entry : renderLayers) {
        if (entry.second->hasTransition()) {
            return true;
        }
    }

    if (placementController.hasTransitions(timePoint)) {
        return true;
    }

    for (const auto& entry : renderSources) {
        if (entry.second->hasFadingTiles()) {
            return true;
        }
    }

    return false;
}

bool RenderOrchestrator::isLoaded() const {
    // do the simple boolean check before iterating over all the tiles in all the sources
    if (!imageManager->isLoaded()) {
        return false;
    }

    for (const auto& entry : renderSources) {
        if (!entry.second->isLoaded()) {
            return false;
        }
    }

    return true;
}

void RenderOrchestrator::clearData() {
    if (!sourceImpls->empty()) sourceImpls = makeMutable<std::vector<Immutable<style::Source::Impl>>>();
    if (!layerImpls->empty()) layerImpls = makeMutable<std::vector<Immutable<style::Layer::Impl>>>();
    if (!imageImpls->empty()) imageImpls = makeMutable<std::vector<Immutable<style::Image::Impl>>>();

    renderSources.clear();
    renderLayers.clear();

    crossTileSymbolIndex.reset();

    if (!lineAtlas->isEmpty()) lineAtlas = std::make_unique<LineAtlas>();
    if (!patternAtlas->isEmpty()) patternAtlas = std::make_unique<PatternAtlas>();

    imageManager->clear();
    glyphManager->evict(fontStacks(*layerImpls));
}

#if MLN_DRAWABLE_RENDERER
void RenderOrchestrator::addChanges(UniqueChangeRequestVec& changes) {
    pendingChanges.insert(
        pendingChanges.end(), std::make_move_iterator(changes.begin()), std::make_move_iterator(changes.end()));
    changes.clear();
}

void RenderOrchestrator::onRemoveLayerGroup(LayerGroupBase&) {}

void RenderOrchestrator::updateLayerGroupOrder() {
    // In the event layer indices change for layer groups, we must re-sort them
    LayerGroupMap newMap;
    for (auto& it : layerGroupsByLayerIndex) {
        newMap.emplace(it.second->getLayerIndex(), it.second);
    }
    layerGroupsByLayerIndex = std::move(newMap);
    layerGroupOrderDirty = false;
}

bool RenderOrchestrator::addLayerGroup(LayerGroupBasePtr layerGroup, const bool replace) {
    const auto index = layerGroup->getLayerIndex();
    const auto result = layerGroupsByLayerIndex.insert(std::make_pair(index, LayerGroupBasePtr{}));
    if (result.second) {
        // added
        result.first->second = std::move(layerGroup);
        return true;
    } else {
        // not added
        if (replace) {
            onRemoveLayerGroup(*result.first->second);
            result.first->second = std::move(layerGroup);
            return true;
        } else {
            return false;
        }
    }
}

bool RenderOrchestrator::removeLayerGroup(const int32_t layerIndex) {
    const auto hit = layerGroupsByLayerIndex.find(layerIndex);
    if (hit != layerGroupsByLayerIndex.end()) {
        onRemoveLayerGroup(*hit->second);
        layerGroupsByLayerIndex.erase(hit);
        return true;
    } else {
        return false;
    }
}

size_t RenderOrchestrator::numLayerGroups() const noexcept {
    return layerGroupsByLayerIndex.size();
}

static const LayerGroupBasePtr no_group;

const LayerGroupBasePtr& RenderOrchestrator::getLayerGroup(const int32_t layerIndex) const {
    const auto hit = layerGroupsByLayerIndex.find(layerIndex);
    return (hit == layerGroupsByLayerIndex.end()) ? no_group : hit->second;
}

void RenderOrchestrator::observeLayerGroups(std::function<void(LayerGroupBase&)> f) {
    for (auto& pair : layerGroupsByLayerIndex) {
        if (pair.second) {
            f(*pair.second);
        }
    }
}

void RenderOrchestrator::observeLayerGroups(std::function<void(const LayerGroupBase&)> f) const {
    for (const auto& pair : layerGroupsByLayerIndex) {
        if (pair.second) {
            f(*pair.second);
        }
    }
}

void RenderOrchestrator::updateLayers(gfx::ShaderRegistry& shaders,
                                      gfx::Context& context,
                                      const TransformState& state,
                                      const std::shared_ptr<UpdateParameters>& updateParameters,
                                      const RenderTree& renderTree) {
    const bool isMapModeContinuous = updateParameters->mode == MapMode::Continuous;
    const auto transitionOptions = isMapModeContinuous ? updateParameters->transitionOptions
                                                       : style::TransitionOptions();
    const auto defDuration = isMapModeContinuous ? util::DEFAULT_TRANSITION_DURATION : Duration::zero();
    const PropertyEvaluationParameters evalParameters{
        getZoomHistory(),
        updateParameters->timePoint,
        transitionOptions.duration.value_or(defDuration),
    };

    std::vector<std::unique_ptr<ChangeRequest>> changes;
    for (const auto& item : renderTree.getLayerRenderItems()) {
        auto& renderLayer = static_cast<const LayerRenderItem&>(item.get()).layer.get();
        renderLayer.update(shaders, context, state, renderTree, changes);
    }
    addChanges(changes);
}

void RenderOrchestrator::processChanges() {
    auto localChanges = std::move(pendingChanges);
    for (auto& change : localChanges) {
        change->execute(*this);
    }

    if (layerGroupOrderDirty) {
        updateLayerGroupOrder();
    }
}

void RenderOrchestrator::markLayerGroupOrderDirty() {
    layerGroupOrderDirty = true;
}

bool RenderOrchestrator::addRenderTarget(RenderTargetPtr renderTarget) {
    auto it = std::find(renderTargets.begin(), renderTargets.end(), renderTarget);
    if (it == renderTargets.end()) {
        renderTargets.emplace_back(renderTarget);
        return true;
    } else {
        return false;
    }
}

bool RenderOrchestrator::removeRenderTarget(const RenderTargetPtr& renderTarget) {
    auto it = std::find(renderTargets.begin(), renderTargets.end(), renderTarget);
    if (it != renderTargets.end()) {
        renderTargets.erase(it);
        return true;
    } else {
        return false;
    }
}

void RenderOrchestrator::observeRenderTargets(std::function<void(RenderTarget&)> f) {
    for (auto& renderTarget : renderTargets) {
        f(*renderTarget);
    }
}

void RenderOrchestrator::observeRenderTargets(std::function<void(const RenderTarget&)> f) const {
    for (const auto& renderTarget : renderTargets) {
        f(*renderTarget);
    }
}
#endif // MLN_DRAWABLE_RENDERER

void RenderOrchestrator::onGlyphsError(const FontStack& fontStack,
                                       const GlyphRange& glyphRange,
                                       std::exception_ptr error) {
    Log::Error(Event::Style,
               "Failed to load glyph range " + std::to_string(glyphRange.first) + "-" +
                   std::to_string(glyphRange.second) + " for font stack " + fontStackToString(fontStack) + ": " +
                   util::toString(error));
    observer->onResourceError(error);
}

void RenderOrchestrator::onTileError(RenderSource& source, const OverscaledTileID& tileID, std::exception_ptr error) {
    Log::Error(Event::Style,
               "Failed to load tile " + util::toString(tileID) + " for source " + source.baseImpl->id + ": " +
                   util::toString(error));
    observer->onResourceError(error);
}

void RenderOrchestrator::onTileChanged(RenderSource&, const OverscaledTileID&) {
    observer->onInvalidate();
}

void RenderOrchestrator::onStyleImageMissing(const std::string& id, const std::function<void()>& done) {
    observer->onStyleImageMissing(id, done);
}

void RenderOrchestrator::onRemoveUnusedStyleImages(const std::vector<std::string>& unusedImageIDs) {
    observer->onRemoveUnusedStyleImages(unusedImageIDs);
}

} // namespace mbgl
