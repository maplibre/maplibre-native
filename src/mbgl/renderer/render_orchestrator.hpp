#pragma once

#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/renderer/render_source_observer.hpp>
#include <mbgl/renderer/render_light.hpp>
#include <mbgl/style/image.hpp>
#include <mbgl/style/source.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/map/zoom_history.hpp>
#include <mbgl/text/cross_tile_symbol_index.hpp>
#include <mbgl/text/glyph_manager_observer.hpp>
#include <mbgl/renderer/image_manager_observer.hpp>
#include <mbgl/text/placement.hpp>
#include <mbgl/renderer/render_tree.hpp>

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mbgl {
class ChangeRequest;
class RendererObserver;
class RenderSource;
class UpdateParameters;
class RenderStaticData;
class RenderedQueryOptions;
class SourceQueryOptions;
class GlyphManager;
class ImageManager;
class LineAtlas;
class PatternAtlas;
class CrossTileSymbolIndex;
class RenderTree;

namespace gfx {
class ShaderRegistry;
class Drawable;
using DrawablePtr = std::shared_ptr<Drawable>;
class DynamicTextureAtlas;
using DynamicTextureAtlasPtr = std::shared_ptr<gfx::DynamicTextureAtlas>;
} // namespace gfx

namespace style {
class LayerProperties;
} // namespace style

using ImmutableLayer = Immutable<style::Layer::Impl>;

class RenderOrchestrator final : public GlyphManagerObserver, public ImageManagerObserver, public RenderSourceObserver {
public:
    RenderOrchestrator(bool backgroundLayerAsColor_,
                       TaggedScheduler& threadPool_,
                       const std::optional<std::string>& localFontFamily_);
    ~RenderOrchestrator() override;

#if MLN_RENDER_BACKEND_OPENGL
    void enableAndroidEmulatorGoldfishMitigation(bool enable) { androidGoldfishMitigationEnabled = enable; }
#endif

    void markContextLost() { contextLost = true; };
    // TODO: Introduce RenderOrchestratorObserver.
    void setObserver(RendererObserver*);

    std::unique_ptr<RenderTree> createRenderTree(const std::shared_ptr<UpdateParameters>&, gfx::DynamicTextureAtlasPtr);

    std::vector<Feature> queryRenderedFeatures(const ScreenLineString&, const RenderedQueryOptions&) const;
    std::vector<Feature> querySourceFeatures(const std::string& sourceID, const SourceQueryOptions&) const;
    std::vector<Feature> queryShapeAnnotations(const ScreenLineString&) const;

    FeatureExtensionValue queryFeatureExtensions(const std::string& sourceID,
                                                 const Feature& feature,
                                                 const std::string& extension,
                                                 const std::string& extensionField,
                                                 const std::optional<std::map<std::string, Value>>& args) const;

    void setFeatureState(const std::string& sourceID,
                         const std::optional<std::string>& layerID,
                         const std::string& featureID,
                         const FeatureState& state);

    void getFeatureState(FeatureState& state,
                         const std::string& sourceID,
                         const std::optional<std::string>& layerID,
                         const std::string& featureID) const;

    void removeFeatureState(const std::string& sourceID,
                            const std::optional<std::string>& sourceLayerID,
                            const std::optional<std::string>& featureID,
                            const std::optional<std::string>& stateKey);

    void setTileCacheEnabled(bool);
    bool getTileCacheEnabled() const;
    void reduceMemoryUse();
    void dumpDebugLogs();
    void collectPlacedSymbolData(bool);
    const std::vector<PlacedSymbolData>& getPlacedSymbolsData() const;
    void clearData();

    void update(const std::shared_ptr<UpdateParameters>&);

    bool addLayerGroup(LayerGroupBasePtr);
    bool removeLayerGroup(const LayerGroupBasePtr&);
    size_t numLayerGroups() const noexcept;
    void updateLayerIndex(LayerGroupBasePtr, int32_t newIndex);

    template <typename Func /* void(LayerGroupBase&) */>
    void visitLayerGroups(Func f) {
        for (auto& pair : layerGroupsByLayerIndex) {
            if (pair.second) {
                f(*pair.second);
            }
        }
    }

    template <typename Func /* void(LayerGroupBase&) */>
    void visitLayerGroupsReversed(Func f) {
        for (auto rit = layerGroupsByLayerIndex.rbegin(); rit != layerGroupsByLayerIndex.rend(); ++rit) {
            if (rit->second) {
                f(*rit->second);
            }
        }
    }

    void updateLayers(gfx::ShaderRegistry&,
                      gfx::Context&,
                      const TransformState&,
                      const std::shared_ptr<UpdateParameters>&,
                      const RenderTree&);

    void processChanges();

    bool addRenderTarget(RenderTargetPtr);
    bool removeRenderTarget(const RenderTargetPtr&);

    template <typename Func /* void(RenderTarget&) */>
    void visitRenderTargets(Func f) {
        for (auto& renderTarget : renderTargets) {
            f(*renderTarget);
        }
    }

    void updateDebugLayerGroups(const RenderTree& renderTree, PaintParameters& parameters);

    template <typename Func /* void(LayerGroupBase&) */>
    void visitDebugLayerGroups(Func f) {
        for (auto& pair : debugLayerGroups) {
            if (pair.second) {
                f(*pair.second);
            }
        }
    }

    const ZoomHistory& getZoomHistory() const { return zoomHistory; }

private:
    bool isLoaded() const;
    bool hasTransitions(TimePoint) const;

    RenderSource* getRenderSource(const std::string& id) const;

    RenderLayer* getRenderLayer(const std::string& id);
    const RenderLayer* getRenderLayer(const std::string& id) const;

    void queryRenderedSymbols(std::unordered_map<std::string, std::vector<Feature>>& resultsByLayer,
                              const ScreenLineString& geometry,
                              const std::unordered_map<std::string, const RenderLayer*>& layers,
                              const RenderedQueryOptions& options) const;

    std::vector<Feature> queryRenderedFeatures(const ScreenLineString&,
                                               const RenderedQueryOptions&,
                                               const std::unordered_map<std::string, const RenderLayer*>&) const;

    // GlyphManagerObserver implementation.
    void onGlyphsLoaded(const FontStack&, const GlyphRange&) override;
    void onGlyphsError(const FontStack&, const GlyphRange&, std::exception_ptr) override;
    void onGlyphsRequested(const FontStack&, const GlyphRange&) override;
    // RenderSourceObserver implementation.
    void onTileChanged(RenderSource&, const OverscaledTileID&) override;
    void onTileError(RenderSource&, const OverscaledTileID&, std::exception_ptr) override;
    void onTileAction(RenderSource&, TileOperation, const OverscaledTileID&, const std::string&) override;

    // ImageManagerObserver implementation
    void onStyleImageMissing(const std::string&, const std::function<void()>&) override;
    void onRemoveUnusedStyleImages(const std::vector<std::string>&) override;

    /// Move changes into the pending set, clearing the provided collection
    void addChanges(UniqueChangeRequestVec&);

    RendererObserver* observer;

    ZoomHistory zoomHistory;
    TransformState transformState;

    std::shared_ptr<GlyphManager> glyphManager;
    std::shared_ptr<ImageManager> imageManager;
    std::unique_ptr<LineAtlas> lineAtlas;
    std::unique_ptr<PatternAtlas> patternAtlas;

    Immutable<std::vector<Immutable<style::Image::Impl>>> imageImpls;
    Immutable<std::vector<Immutable<style::Source::Impl>>> sourceImpls;
    Immutable<std::vector<Immutable<style::Layer::Impl>>> layerImpls;

    std::unordered_map<std::string, std::unique_ptr<RenderSource>> renderSources;
    std::unordered_map<std::string, std::unique_ptr<RenderLayer>> renderLayers;
    RenderLight renderLight;

    CrossTileSymbolIndex crossTileSymbolIndex;
    PlacementController placementController;

    const bool backgroundLayerAsColor;
    bool contextLost = false;
    bool placedSymbolDataCollected = false;
    bool tileCacheEnabled = true;

#if MLN_RENDER_BACKEND_OPENGL
    bool androidGoldfishMitigationEnabled{false};
#endif

    // Vectors with reserved capacity of layerImpls->size() to avoid
    // reallocation on each frame.
    std::vector<Immutable<style::LayerProperties>> filteredLayersForSource;
    RenderLayerReferences orderedLayers;
    RenderLayerReferences layersNeedPlacement;

    TaggedScheduler threadPool;

    std::vector<std::unique_ptr<ChangeRequest>> pendingChanges;

    using LayerGroupMap = std::multimap<int32_t, LayerGroupBasePtr>;
    LayerGroupMap layerGroupsByLayerIndex;

    std::vector<RenderTargetPtr> renderTargets;
    RenderItem::DebugLayerGroupMap debugLayerGroups;
};

} // namespace mbgl
