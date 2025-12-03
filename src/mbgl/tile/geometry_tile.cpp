#include <mbgl/tile/geometry_tile.hpp>

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/style/layers/custom_layer.hpp>
#include <mbgl/renderer/layers/render_custom_layer.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/layers/render_background_layer.hpp>
#include <mbgl/renderer/layers/render_symbol_layer.hpp>
#include <mbgl/renderer/query.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/tile/geometry_tile_worker.hpp>
#include <mbgl/tile/tile_observer.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/thread_pool.hpp>
#include <mbgl/gfx/upload_pass.hpp>

#include <utility>

namespace mbgl {

LayerRenderData* GeometryTile::LayoutResult::getLayerRenderData(const style::Layer::Impl& layerImpl) {
    MLN_TRACE_FUNC();
    MLN_ZONE_STR(layerImpl.id);

    auto it = layerRenderData.find(layerImpl.id);
    if (it == layerRenderData.end()) {
        return nullptr;
    }
    LayerRenderData& result = it->second;
    if (result.layerProperties->baseImpl->getTypeInfo() != layerImpl.getTypeInfo()) {
        // Layer data might be outdated, see issue #12432.
        return nullptr;
    }
    return &result;
}

class ImagePatch {
public:
    ImagePatch(Immutable<style::Image::Impl> image_, const Rect<uint16_t>& paddedRect_)
        : image(std::move(image_)),
          paddedRect(paddedRect_) {}
    Immutable<style::Image::Impl> image;
    Rect<uint16_t> paddedRect;
};

void populateImagePatches(ImagePositions& imagePositions,
                          const ImageManager& imageManager,
                          std::vector<ImagePatch>& /*out*/ patches) {
    if (imagePositions.empty()) {
        imagePositions.reserve(imageManager.updatedImageVersions.size());
    }
    for (auto& updatedImageVersion : imageManager.updatedImageVersions) {
        const std::string& name = updatedImageVersion.first;
        const uint32_t version = updatedImageVersion.second;
        const auto it = imagePositions.find(updatedImageVersion.first);
        if (it != imagePositions.end()) {
            auto& position = it->second;
            if (position.version == version) continue;

            const auto updatedImage = imageManager.getSharedImage(name);
            if (updatedImage == nullptr) continue;

            patches.emplace_back(*updatedImage, position.paddedRect);
            position.version = version;
        }
    }
}

class GeometryTileRenderData final : public TileRenderData {
public:
    GeometryTileRenderData(std::shared_ptr<GeometryTile::LayoutResult> layoutResult_,
                           std::shared_ptr<TileAtlasTextures> atlasTextures_)
        : TileRenderData(std::move(atlasTextures_)),
          layoutResult(std::move(layoutResult_)) {}

private:
    // TileRenderData overrides.
    std::optional<ImagePosition> getPattern(const std::string&) const override;
    const LayerRenderData* getLayerRenderData(const style::Layer::Impl&) const override;
    Bucket* getBucket(const style::Layer::Impl&) const override;
    void upload(gfx::UploadPass&) override;
    void prepare(const SourcePrepareParameters&) override;

    std::shared_ptr<GeometryTile::LayoutResult> layoutResult;
    std::vector<ImagePatch> imagePatches;
};

using namespace style;

std::optional<ImagePosition> GeometryTileRenderData::getPattern(const std::string& pattern) const {
    MLN_TRACE_FUNC();

    if (layoutResult) {
        const auto& patternPositions = layoutResult->imageAtlas.patternPositions;
        auto it = patternPositions.find(pattern);
        if (it != patternPositions.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}

void GeometryTileRenderData::upload(gfx::UploadPass& uploadPass) {
    MLN_TRACE_FUNC();

    if (!layoutResult) return;

    auto uploadFn = [&](Bucket& bucket) {
        if (bucket.needsUpload()) {
            bucket.upload(uploadPass);
        }
    };

    for (auto& entry : layoutResult->layerRenderData) {
        uploadFn(*entry.second.bucket);
    }

    assert(atlasTextures);

    if (const auto& glyphDynamicTexture = layoutResult->glyphAtlas.dynamicTexture) {
        glyphDynamicTexture->uploadDeferredImages();
        atlasTextures->glyph = glyphDynamicTexture->getTexture();
    }
    if (const auto& imageDynamicTexture = layoutResult->imageAtlas.dynamicTexture) {
        imageDynamicTexture->uploadDeferredImages();
        atlasTextures->icon = imageDynamicTexture->getTexture();
    }

    if (!imagePatches.empty()) {
        for (const auto& imagePatch : imagePatches) { // patch updated images.
            atlasTextures->icon->uploadSubRegion(imagePatch.image->image,
                                                 imagePatch.paddedRect.x + ImagePosition::padding,
                                                 imagePatch.paddedRect.y + ImagePosition::padding);
        }
        imagePatches.clear();
    }
}

void GeometryTileRenderData::prepare(const SourcePrepareParameters& parameters) {
    MLN_TRACE_FUNC();

    if (!layoutResult) return;
    imagePatches.clear();
    populateImagePatches(layoutResult->imageAtlas.iconPositions, parameters.imageManager, imagePatches);
    populateImagePatches(layoutResult->imageAtlas.patternPositions, parameters.imageManager, imagePatches);
}

Bucket* GeometryTileRenderData::getBucket(const Layer::Impl& layer) const {
    const LayerRenderData* data = getLayerRenderData(layer);
    return data ? data->bucket.get() : nullptr;
}

const LayerRenderData* GeometryTileRenderData::getLayerRenderData(const style::Layer::Impl& layerImpl) const {
    return layoutResult ? layoutResult->getLayerRenderData(layerImpl) : nullptr;
}

GeometryTile::LayoutResult::~LayoutResult() {
    if (dynamicTextureAtlas) {
        dynamicTextureAtlas->removeTextures(glyphAtlas.textureHandles, glyphAtlas.dynamicTexture);
        dynamicTextureAtlas->removeTextures(imageAtlas.textureHandles, imageAtlas.dynamicTexture);
    }
}

/*
   Correlation between GeometryTile and GeometryTileWorker is safeguarded by two
   correlation schemes:

   GeometryTile's 'correlationID' is used for ensuring the tile will be flagged
   as non-pending only when the placement coming from the last operation (as in
   'setData', 'setLayers',  'setShowCollisionBoxes') occurs. This is important
   for still mode rendering as we want to render only when all layout and
   placement operations are completed.

   GeometryTileWorker's 'imageCorrelationID' is used for checking whether an
   image request reply coming from `GeometryTile` is valid. Previous image
   request replies are ignored as they result in incomplete placement attempts
   that could flag the tile as non-pending too early.
 */

GeometryTile::GeometryTile(const OverscaledTileID& id_,
                           std::string sourceID_,
                           const TileParameters& parameters,
                           TileObserver* observer_)
    : Tile(Kind::Geometry, id_, std::move(sourceID_), observer_),
      ImageRequestor(parameters.imageManager),
      threadPool(parameters.threadPool),
      mailbox(std::make_shared<Mailbox>(*Scheduler::GetCurrent())),
      worker(parameters.isUpdateSynchronous,
             parameters.threadPool, // Scheduler reference for the Actor retainer
             OptionalActorRef<GeometryTile>(parameters.isUpdateSynchronous, *this, mailbox),
             parameters.threadPool,
             id_,
             sourceID,
             obsolete,
             parameters.mode,
             parameters.pixelRatio,
             parameters.debugOptions & MapDebugOptions::Collision,
             parameters.dynamicTextureAtlas,
             parameters.glyphManager->getFontFaces()),
      fileSource(parameters.fileSource),
      glyphManager(parameters.glyphManager),
      imageManager(parameters.imageManager),
      mode(parameters.mode),
      showCollisionBoxes(parameters.debugOptions & MapDebugOptions::Collision) {}

GeometryTile::~GeometryTile() {
    MLN_TRACE_FUNC();

    markObsolete();

    glyphManager->removeRequestor(*this);
    imageManager->removeRequestor(*this);

    if (pending) {
        // This tile never finished loading or was abandoned, emit a cancellation event
        observer->onTileAction(id, sourceID, TileOperation::Cancelled);
    }

    if (layoutResult) {
        threadPool.runOnRenderThread(
            [layoutResult_{std::move(layoutResult)}, atlasTextures_{std::move(atlasTextures)}]() {});
    }
}

void GeometryTile::cancel() {
    markObsolete();
}

void GeometryTile::markObsolete() {
    obsolete = true;
    mailbox->abandon();
}

void GeometryTile::setError(std::exception_ptr err) {
    loaded = true;
    observer->onTileError(*this, std::move(err));
}

void GeometryTile::setData(std::unique_ptr<const GeometryTileData> data_) {
    MLN_TRACE_FUNC();

    if (obsolete) {
        return;
    }

    if (!pending) {
        observer->onTileAction(id, sourceID, TileOperation::StartParse);
    }

    // Mark the tile as pending again if it was complete before to prevent
    // signaling a complete state despite pending parse operations.
    pending = true;

    ++correlationID;
    worker.self().invoke(
        &GeometryTileWorker::setData, std::move(data_), imageManager->getAvailableImages(), correlationID);
}

void GeometryTile::reset() {
    MLN_TRACE_FUNC();

    // If there is pending work, indicate that work has been cancelled.
    // Clear the pending status.
    if (pending) {
        observer->onTileAction(id, sourceID, TileOperation::Cancelled);
        pending = false;
    }

    // Reset the tile to an unloaded state to avoid signaling completion
    // after clearing the tile's pending status.
    loaded = false;

    // Reset the worker to the `NeedsParse` state.
    ++correlationID;
    worker.self().invoke(&GeometryTileWorker::reset, correlationID);
}

std::unique_ptr<TileRenderData> GeometryTile::createRenderData() {
    MLN_TRACE_FUNC();

    return std::make_unique<GeometryTileRenderData>(layoutResult, atlasTextures);
}

void GeometryTile::setLayers(const std::vector<Immutable<LayerProperties>>& layers) {
    MLN_TRACE_FUNC();

    // Mark the tile as pending again if it was complete before to prevent
    // signaling a complete state despite pending parse operations.
    if (!pending) {
        pending = true;
        observer->onTileAction(id, sourceID, TileOperation::StartParse);
    }

    std::vector<Immutable<LayerProperties>> impls;
    impls.reserve(layers.size());

    for (const auto& layer : layers) {
        MLN_TRACE_ZONE(layer);
        MLN_ZONE_STR(layer->baseImpl->id);
        // Skip irrelevant layers.
        const auto& layerImpl = *layer->baseImpl;
        assert(layerImpl.getTypeInfo()->source != LayerTypeInfo::Source::NotRequired);
        assert(layerImpl.source == sourceID);
        assert(layerImpl.visibility != VisibilityType::None);
        if (id.overscaledZ < std::floor(layerImpl.minZoom) || id.overscaledZ >= std::ceil(layerImpl.maxZoom)) {
            continue;
        }

        impls.push_back(layer);
    }

    ++correlationID;
    worker.self().invoke(
        &GeometryTileWorker::setLayers, std::move(impls), imageManager->getAvailableImages(), correlationID);
}

void GeometryTile::setShowCollisionBoxes(const bool showCollisionBoxes_) {
    MLN_TRACE_FUNC();

    if (showCollisionBoxes != showCollisionBoxes_) {
        showCollisionBoxes = showCollisionBoxes_;
        ++correlationID;
        worker.self().invoke(&GeometryTileWorker::setShowCollisionBoxes, showCollisionBoxes, correlationID);
    }
}

void GeometryTile::onLayout(std::shared_ptr<LayoutResult> result, const uint64_t resultCorrelationID) {
    MLN_TRACE_FUNC();

    loaded = true;
    renderable = true;
    if (resultCorrelationID == correlationID) {
        pending = false;
        observer->onTileAction(id, sourceID, TileOperation::EndParse);
    }

    layoutResult = std::move(result);
    if (!atlasTextures) {
        atlasTextures = std::make_shared<TileAtlasTextures>();
    }

    if (layoutResult) {
        for (const auto& data : layoutResult->layerRenderData) {
            if (data.second.bucket) {
                data.second.bucket->check(SYM_GUARD_LOC);
            }
        }
    }

    observer->onTileChanged(*this);

    if (layoutResult) {
        for (const auto& data : layoutResult->layerRenderData) {
            if (data.second.bucket) {
                data.second.bucket->check(SYM_GUARD_LOC);
            }
        }
    }
}

void GeometryTile::onError(std::exception_ptr err, const uint64_t resultCorrelationID) {
    loaded = true;
    if (resultCorrelationID == correlationID) {
        pending = false;
        observer->onTileAction(id, sourceID, TileOperation::Error);
    }
    observer->onTileError(*this, std::move(err));
}

void GeometryTile::onGlyphsAvailable(GlyphMap glyphMap, [[maybe_unused]] HBShapeRequests requests) {
    MLN_TRACE_FUNC();

    HBShapeResults results;
#ifdef MLN_TEXT_SHAPING_HARFBUZZ
    for (auto& fontStackIT : requests) {
        auto fontStack = fontStackIT.first;
        auto& fontTypes = fontStackIT.second;
        for (auto& typesIT : fontTypes) {
            auto type = typesIT.first;
            auto& strs = typesIT.second;

            for (auto& str : strs) {
                std::vector<GlyphID> shapedGlyphIDs;
                std::shared_ptr<std::vector<HBShapeAdjust>> shapedAdjusts =
                    std::make_shared<std::vector<HBShapeAdjust>>();
                glyphManager->hbShaping(str, fontStack, type, shapedGlyphIDs, *shapedAdjusts);
                std::u16string shapedstr;

                shapedstr.reserve(shapedGlyphIDs.size());
                for (auto& glyphID : shapedGlyphIDs) {
                    shapedstr += glyphID.complex.code;

                    auto fontStackHash = FontStackHasher()(fontStack);
                    bool needShape = true;
                    if (glyphMap.find(fontStackHash) != glyphMap.end()) {
                        auto& glyphs = glyphMap[fontStackHash];
                        if (glyphs.find(glyphID) != glyphs.end()) needShape = false;
                    }
                    if (needShape) {
                        auto glyph = glyphManager->getGlyph(fontStack, glyphID);
                        glyphMap[fontStackHash].emplace(glyph->id, glyph);
                    }
                }

                results[fontStack][type][str] = HBShapeResult{shapedstr,
                                                              shapedAdjusts}; //.emplace(str, shapedstr, shapedAdjusts);
            }
        }
    }
#endif // MLN_TEXT_SHAPING_HARFBUZZ
    worker.self().invoke(&GeometryTileWorker::onGlyphsAvailable, std::move(glyphMap), std::move(results));
}

void GeometryTile::getGlyphs(GlyphDependencies glyphDependencies) {
    MLN_TRACE_FUNC();

    if (fileSource) {
        glyphManager->getGlyphs(*this, std::move(glyphDependencies), *fileSource);
    }
}

void GeometryTile::onImagesAvailable(ImageMap images,
                                     ImageMap patterns,
                                     ImageVersionMap versionMap,
                                     uint64_t imageCorrelationID) {
    MLN_TRACE_FUNC();

    worker.self().invoke(&GeometryTileWorker::onImagesAvailable,
                         std::move(images),
                         std::move(patterns),
                         std::move(versionMap),
                         imageCorrelationID);
}

void GeometryTile::getImages(ImageRequestPair pair) {
    MLN_TRACE_FUNC();

    imageManager->getImages(*this, std::move(pair));
}

std::shared_ptr<FeatureIndex> GeometryTile::getFeatureIndex() const {
    return layoutResult ? layoutResult->featureIndex : nullptr;
}

bool GeometryTile::layerPropertiesUpdated(const Immutable<style::LayerProperties>& layerProperties) {
    MLN_TRACE_FUNC();

    LayerRenderData* renderData = getLayerRenderData(*layerProperties->baseImpl);
    if (!renderData) {
        return false;
    }

    if (renderData->layerProperties != layerProperties &&
        renderData->layerProperties->constantsMask() == layerProperties->constantsMask()) {
        renderData->layerProperties = layerProperties;
    }

    return true;
}

const GeometryTileData* GeometryTile::getData() const {
    if (!layoutResult || !layoutResult->featureIndex) {
        return nullptr;
    }
    return layoutResult->featureIndex->getData();
}

LayerRenderData* GeometryTile::getLayerRenderData(const style::Layer::Impl& layerImpl) {
    MLN_TRACE_FUNC();

    return layoutResult ? layoutResult->getLayerRenderData(layerImpl) : nullptr;
}

float GeometryTile::getQueryPadding(const std::unordered_map<std::string, const RenderLayer*>& layers) {
    MLN_TRACE_FUNC();

    float queryPadding = 0;
    for (const auto& pair : layers) {
        const LayerRenderData* data = getLayerRenderData(*pair.second->baseImpl);
        if (data && data->bucket && data->bucket->hasData()) {
            queryPadding = std::max(queryPadding, data->bucket->getQueryRadius(*pair.second));
        }
    }
    return queryPadding;
}

void GeometryTile::queryRenderedFeatures(std::unordered_map<std::string, std::vector<Feature>>& result,
                                         const GeometryCoordinates& queryGeometry,
                                         const TransformState& transformState,
                                         const std::unordered_map<std::string, const RenderLayer*>& layers,
                                         const RenderedQueryOptions& options,
                                         const mat4& projMatrix,
                                         const SourceFeatureState& featureState) {
    MLN_TRACE_FUNC();

    if (!getData()) return;

    const float queryPadding = getQueryPadding(layers);

    mat4 posMatrix;
    transformState.matrixFor(posMatrix, id.toUnwrapped());
    matrix::multiply(posMatrix, projMatrix, posMatrix);

    layoutResult->featureIndex->query(result,
                                      queryGeometry,
                                      transformState,
                                      posMatrix,
                                      util::tileSize_D * id.overscaleFactor(),
                                      std::pow(2, transformState.getZoom() - id.overscaledZ),
                                      options,
                                      id.toUnwrapped(),
                                      layers,
                                      queryPadding * transformState.maxPitchScaleFactor(),
                                      featureState);
}

void GeometryTile::querySourceFeatures(std::vector<Feature>& result, const SourceQueryOptions& options) {
    MLN_TRACE_FUNC();

    // Data not yet available, or tile is empty
    if (!getData()) {
        return;
    }

    // No source layers, specified, nothing to do
    if (!options.sourceLayers) {
        Log::Warning(Event::General, "At least one sourceLayer required");
        return;
    }

    for (const auto& sourceLayer : *options.sourceLayers) {
        // Go throught all sourceLayers, if any
        // to gather all the features
        auto layer = getData()->getLayer(sourceLayer);

        if (layer) {
            auto featureCount = layer->featureCount();
            for (std::size_t i = 0; i < featureCount; i++) {
                auto feature = layer->getFeature(i);

                // Apply filter, if any
                if (options.filter && !(*options.filter)(style::expression::EvaluationContext{
                                          static_cast<float>(this->id.overscaledZ), feature.get()})) {
                    continue;
                }

                result.emplace_back(convertFeature(*feature, id.canonical));
            }
        }
    }
}

bool GeometryTile::holdForFade() const {
    return mode == MapMode::Continuous &&
           (fadeState == FadeState::NeedsFirstPlacement || fadeState == FadeState::NeedsSecondPlacement);
}

void GeometryTile::markRenderedIdeal() {
    fadeState = FadeState::Loaded;
}
void GeometryTile::markRenderedPreviously() {
    if (fadeState == FadeState::Loaded) {
        fadeState = FadeState::NeedsFirstPlacement;
    }
}
void GeometryTile::performedFadePlacement() {
    if (fadeState == FadeState::NeedsFirstPlacement) {
        fadeState = FadeState::NeedsSecondPlacement;
    } else if (fadeState == FadeState::NeedsSecondPlacement) {
        fadeState = FadeState::CanRemove;
    }
}

void GeometryTile::setFeatureState(const LayerFeatureStates& states) {
    MLN_TRACE_FUNC();

    const auto layers = getData();
    if ((layers == nullptr) || states.empty() || !layoutResult) {
        return;
    }

    for (auto& layerIdToLayerRenderData = layoutResult->layerRenderData;
         auto& [layerID, renderData] : layerIdToLayerRenderData) {
        std::string sourceLayerId = renderData.layerProperties->baseImpl->sourceLayer;
        if (const auto sourceLayer = layers->getLayer(sourceLayerId)) {
            auto entry = states.find(sourceLayerId);
            if (entry == states.end()) {
                continue;
            }
            const auto& featureStates = entry->second;
            if (featureStates.empty()) {
                continue;
            }
            if (const auto bucket = renderData.bucket; bucket && bucket->hasData()) {
                bucket->update(featureStates, *sourceLayer, layerID, layoutResult->imageAtlas.patternPositions);
            }
        }
    }
}

} // namespace mbgl
