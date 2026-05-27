#pragma once
#include <mbgl/layout/layout.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/util/mat4.hpp>

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/util/tiny_unordered_map.hpp>

#include <list>
#include <memory>
#include <string>

namespace mbgl {
class Bucket;
class DynamicFeatureIndex;
class LineAtlas;
class PropertyEvaluationParameters;
class PaintParameters;
class PatternAtlas;
class RenderTile;
class RenderTree;
class SymbolBucket;
class TransformState;
class TransitionParameters;
class UpdateParameters;
class UploadParameters;

class ChangeRequest;
using LayerGroupBasePtr = std::shared_ptr<LayerGroupBase>;
using UniqueChangeRequest = std::unique_ptr<ChangeRequest>;
using UniqueChangeRequestVec = std::vector<UniqueChangeRequest>;

namespace gfx {
class Context;
class ShaderGroup;
class ShaderRegistry;
using ShaderGroupPtr = std::shared_ptr<ShaderGroup>;

class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

namespace style {
class ColorRampPropertyValue;
} // namespace style

class LayerRenderData {
public:
    std::shared_ptr<Bucket> bucket;
    Immutable<style::LayerProperties> layerProperties;
};

class SortKeyRange {
public:
    bool isFirstRange() const { return start == 0u; }
    float sortKey;
    size_t start;
    size_t end;
};

class BucketPlacementData {
public:
    std::reference_wrapper<Bucket> bucket;
    std::reference_wrapper<const RenderTile> tile;
    std::shared_ptr<FeatureIndex> featureIndex;
    std::string sourceId;
    std::optional<SortKeyRange> sortKeyRange;
};

using LayerPlacementData = std::list<BucketPlacementData>;

class LayerPrepareParameters {
public:
    RenderSource* source;
    ImageManager& imageManager;
    PatternAtlas& patternAtlas;
    LineAtlas& lineAtlas;
    const TransformState& state;
};

class RenderLayer {
protected:
    RenderLayer(Immutable<style::LayerProperties>);

public:
    virtual ~RenderLayer() = default;

    // Begin transitions for any properties that have changed since the last frame.
    virtual void transition(const TransitionParameters&) = 0;

    // Overloaded version for transitions to a new layer impl.
    void transition(const TransitionParameters&, Immutable<style::Layer::Impl> newImpl);

    // Fully evaluate possibly-transitioning paint properties based on a zoom
    // level. Updates the contained `evaluatedProperties` member.
    virtual void evaluate(const PropertyEvaluationParameters&) = 0;

    // Returns true if any paint properties have active transitions.
    virtual bool hasTransition() const = 0;

    // Returns true if the layer has a pattern property and is actively crossfading.
    virtual bool hasCrossfade() const = 0;

    // Returns true if layer writes to depth buffer by drawing using PaintParameters::depthModeFor3D().
    virtual bool is3D() const { return false; }

    // Returns true is the layer is subject to placement.
    bool needsPlacement() const;

    const std::string& getID() const;

    int32_t getLayerIndex() const noexcept;

    // Checks whether this layer needs to be rendered in the given render pass.
    bool hasRenderPass(RenderPass) const;

    // Checks whether this layer can be rendered.
    bool needsRendering() const;

    // Checks whether the given zoom is inside this layer zoom range.
    bool supportsZoom(float zoom) const;

    virtual void upload(gfx::UploadPass&) {}
    virtual void render(PaintParameters&) {}

    // Check whether the given geometry intersects with the feature
    virtual bool queryIntersectsFeature(const GeometryCoordinates&,
                                        const GeometryTileFeature&,
                                        const float,
                                        const TransformState&,
                                        const float,
                                        const mat4&,
                                        const FeatureState&) const {
        return false;
    };

    virtual void populateDynamicRenderFeatureIndex(DynamicFeatureIndex&) const {}

    virtual void prepare(const LayerPrepareParameters&);

    const LayerPlacementData& getPlacementData() const { return placementData; }

    // Latest evaluated properties.
    Immutable<style::LayerProperties> evaluatedProperties;
    // Private implementation
    Immutable<style::Layer::Impl> baseImpl;

    virtual void markContextDestroyed();

    // TODO: Only for background layers.
    virtual std::optional<Color> getSolidBackground() const;

    /// Generate any changes needed by the layer
    virtual void update(gfx::ShaderRegistry&,
                        gfx::Context&,
                        const TransformState&,
                        const std::shared_ptr<UpdateParameters>&,
                        const RenderTree&,
                        UniqueChangeRequestVec&) {}

    /// Called when the style layer is replaced (same ID and type), and the render layer is reused.
    virtual void layerChanged(const TransitionParameters&,
                              const Immutable<style::Layer::Impl>& newLayer,
                              UniqueChangeRequestVec&);

    /// Called when the style layer is removed
    virtual void layerRemoved(UniqueChangeRequestVec&);

    /// @brief Called when the layer index changes
    /// @param newLayerIndex The new layer index for this layer
    /// @param changes The collection of current pending change requests
    virtual void layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec& changes);

    /// @brief Called by the RenderOrchestrator during RenderTree construction.
    /// This event is run to indicate if the layer should render or not for the current frame.
    /// @param willRender Indicates if this layer should render or not
    /// @param changes The collection of current pending change requests
    virtual void markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes);

    /// Returns the current renderability mode of the layer
    bool isLayerRenderable() const noexcept { return isRenderable; }

    /// Remove all the drawables for tiles
    virtual std::size_t removeAllDrawables();

    using Dependency = style::expression::Dependency;
    Dependency getStyleDependencies() const { return styleDependencies; }

protected:
    // Checks whether the current hardware can render this layer. If it can't,
    // we'll show a warning in the console to inform the developer.
    void checkRenderability(const PaintParameters&, uint32_t activeBindingCount);

    void addRenderPassesFromTiles();

    const LayerRenderData* getRenderDataForPass(const RenderTile&, RenderPass) const;

    void setLayerGroup(LayerGroupBasePtr, UniqueChangeRequestVec&);

    /// (Un-)Register the layer group with the orchestrator
    void activateLayerGroup(const LayerGroupBasePtr&, bool activate, UniqueChangeRequestVec& changes);

    /// Change the layer index on a layer group associated with this layer
    void changeLayerIndex(const LayerGroupBasePtr&, int32_t newLayerIndex, UniqueChangeRequestVec&);

    /// Update the drawables for a tile.
    /// @param renderPass The pass to consider
    /// @param tileID The tile to consider
    /// @param updateFunction A function that updates a single drawable.  Should return true if the drawable
    ///                       was updated or false if it was skipped because it's for a previous style.
    /// @return true if drawables were updated
    template <typename Func /* bool(gfx::Drawable&) */>
    bool updateTile(RenderPass renderPass, const OverscaledTileID& tileID, Func update) {
        bool anyUpdated = false;
        if (const auto tileGroup = static_cast<TileLayerGroup*>(layerGroup.get())) {
            bool unUpdatedDrawables = false;
            tileGroup->visitDrawables(renderPass, tileID, [&](gfx::Drawable& drawable) {
                if (update(drawable)) {
                    anyUpdated = true;
                } else {
                    unUpdatedDrawables = true;
                }
            });

            // If any are updated, the caller shouldn't add new ones.
            // If none are updated and some were skipped, remove those.
            // This is to handle the case that the style layer changes but the bucket is not re-created.
            if (!anyUpdated && unUpdatedDrawables) {
                removeTile(renderPass, tileID);
            }
        }
        return anyUpdated;
    }

    /// Remove all drawables for the tile from the layer group
    /// @return The number of drawables actually removed.
    virtual std::size_t removeTile(RenderPass, const OverscaledTileID&);

    /// Update `renderTileIDs` from `renderTiles`
    void updateRenderTileIDs();

    /// Whether a given tile ID is present in the current cover set (`renderTiles`)
    bool hasRenderTile(const OverscaledTileID&) const;

    /// Get the bucket ID from which a given tile was built
    /// @details When a new style is loaded and contains a layer with the same ID, `layerChanged` will be called during
    /// style
    ///          parsing, but the `Bucket` in a tile's `RenderData` will only be replaced when the asynchronous load for
    ///          the tile is complete, at which point the drawable for the tile may need to be updated or replaced.
    util::SimpleIdentity getRenderTileBucketID(const OverscaledTileID&) const;

    /// Set the bucket ID from which a given tile was built
    /// @return true if updated, false if the tile ID is not present in the set of tiles to be rendered or the ID is
    /// unchanged
    bool setRenderTileBucketID(const OverscaledTileID&, util::SimpleIdentity bucketID);

    static bool applyColorRamp(const style::ColorRampPropertyValue&, PremultipliedImage&);

protected:
    // Stores current set of tiles to be rendered for this layer.
    RenderTiles renderTiles;

    // Retains ownership of tiles
    Immutable<std::vector<RenderTile>> renderTilesOwner;

    // Stores what render passes this layer is currently enabled for. This depends on the
    // evaluated StyleProperties object and is updated accordingly.
    RenderPass passes = RenderPass::None;

    LayerPlacementData placementData;

    // will need to be overriden to handle their activation.
    LayerGroupBasePtr layerGroup;

    // An optional tweaker that will update drawables
    LayerTweakerPtr layerTweaker;

    // A sorted set of tile IDs in `renderTiles`, along with
    // the identity of the bucket from which they were built.
    // We swap between two instances to minimize reallocations.
    static constexpr auto LinearTileIDs = 12; // From benchmarking, see #1805
    using RenderTileIDMap = util::TinyUnorderedMap<OverscaledTileID, util::SimpleIdentity, LinearTileIDs>;
    RenderTileIDMap renderTileIDs;
    RenderTileIDMap newRenderTileIDs;

    // Current layer index as specified by the layerIndexChanged event
    int32_t layerIndex{0};

    Dependency styleDependencies = Dependency::None;

    // Current renderable status as specified by the markLayerRenderable event
    bool isRenderable{false};

    struct Stats {
        size_t propertyEvaluations = 0;
        size_t drawablesAdded = 0;
        size_t drawablesRemoved = 0;
    } stats;

private:
    // Some layers may not render correctly on some hardware when the vertex
    // attribute limit of that GPU is exceeded. More attributes are used when
    // adding many data driven paint properties to a layer.
    bool hasRenderFailures = false;
};

using RenderLayerReferences = std::vector<std::reference_wrapper<RenderLayer>>;

} // namespace mbgl
