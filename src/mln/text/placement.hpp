#pragma once

#include <mln/layout/symbol_projection.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/style/transition_options.hpp>
#include <mln/text/collision_index.hpp>
#include <mln/util/chrono.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace mln {

class SymbolBucket;
class SymbolInstance;
using SymbolInstanceReferences = std::vector<std::reference_wrapper<const SymbolInstance>>;
class UpdateParameters;
enum class PlacedSymbolOrientation : bool;

class OpacityState {
public:
    OpacityState(bool placed, bool skipFade);
    OpacityState(const OpacityState& prevState, float increment, bool placed);
    bool isHidden() const;
    float opacity;
    bool placed;
};

class JointOpacityState {
public:
    JointOpacityState(bool placedText, bool placedIcon, bool skipFade);
    JointOpacityState(const JointOpacityState& prevOpacityState, float increment, bool placedText, bool placedIcon);
    bool isHidden() const;
    OpacityState icon;
    OpacityState text;
};

class VariableOffset {
public:
    std::array<float, 2> offset;
    float width;
    float height;
    style::TextVariableAnchorType anchor;
    float textBoxScale;
    std::optional<style::TextVariableAnchorType> prevAnchor;
};

class JointPlacement {
public:
    JointPlacement(bool text_, bool icon_, bool skipFade_)
        : text(text_),
          icon(icon_),
          skipFade(skipFade_) {}

    bool placed() const { return text || icon; }

    const bool text;
    const bool icon;
    // skipFade = outside viewport, but within CollisionIndex::viewportPadding
    // px of the edge Because these symbols aren't onscreen yet, we can skip the
    // "fade in" animation, and if a subsequent viewport change brings them into
    // view, they'll be fully visible right away.
    const bool skipFade;
};

struct RetainedQueryData {
    uint32_t bucketInstanceId;
    std::shared_ptr<FeatureIndex> featureIndex;
    OverscaledTileID tileID;
    mutable FeatureSortOrder featureSortOrder;

    RetainedQueryData(uint32_t bucketInstanceId_, std::shared_ptr<FeatureIndex> featureIndex_, OverscaledTileID tileID_)
        : bucketInstanceId(bucketInstanceId_),
          featureIndex(std::move(featureIndex_)),
          tileID(tileID_) {}
};

class CollisionGroups {
public:
    using Predicate = std::function<bool(const RefIndexedSubfeature&)>;
    using CollisionGroup = std::pair<uint16_t, std::optional<Predicate>>;

    CollisionGroups(const bool crossSourceCollisions_)
        : maxGroupID(0),
          crossSourceCollisions(crossSourceCollisions_) {}

    const CollisionGroup& get(const std::string& sourceID);

private:
    std::map<std::string, CollisionGroup> collisionGroups;
    uint16_t maxGroupID;
    bool crossSourceCollisions;
};

/**
 * @brief The reading direction of line labels kept upright, by cross tile id, used with a keep-upright
 * margin (MapOptions::withKeepUprightMargin) so that every copy of a label (tiles of two zoom levels,
 * a tile back from the cache) reads the way the label was last drawn.
 *
 * Between two syncs, each copy keeps its own direction from frame to frame
 * (PlacedSymbol::keepUprightFlipped). Placement syncs a layer when a new placement draws it for the
 * first time, when one of its buckets has just entered the cross tile index, or when its set of buckets
 * changes (see Placement::syncLineLabelDirections), not on every frame: first remember() for every
 * bucket of the layer, then restore() for every bucket of it. Only text is synced, icons are never read
 * nor written.
 *
 * What is shared is the reading direction on the map, not the flag: the copies of a label are matched
 * by text and position only, and the line of one copy may run the other way than the line of another
 * (tiles merge or cut lines differently), so the same reading direction can be drawn flipped in one
 * copy and unflipped in the other.
 */
class LineLabelDirections {
public:
    /// Records the reading direction of each label of the bucket that has been drawn since the last
    /// sync: not hidden, in a bucket that is not new to the cross tile index
    /// (SymbolBucket::lineLabelDirectionsStale).
    void remember(const SymbolBucket&);
    /// Sets the direction of each label of the bucket, drawn or not, to the recorded one, or else to the
    /// one recorded in `older` (the previous placement's), or unsets it: such a label is decided as if it
    /// were drawn for the first time. Clears SymbolBucket::lineLabelDirectionsStale.
    void restore(SymbolBucket&, const LineLabelDirections* older) const;

    /// The direction in which the label reads, in tile coordinates (the axes are the same in the tiles of
    /// every zoom level); its length has no meaning.
    std::optional<Point<float>> readingDirection(uint32_t crossTileID) const;

private:
    std::unordered_map<uint32_t, Point<float>> directions;
};

class Placement;
class PlacementContext;
class PlacementController {
public:
    PlacementController();
    void setPlacement(Immutable<Placement>);
    const Immutable<Placement>& getPlacement() const { return placement; }
    void setPlacementStale() { stale = true; }
    bool placementIsRecent(TimePoint now, float zoom, std::optional<Duration> periodOverride = std::nullopt) const;
    bool hasTransitions(TimePoint now) const;

private:
    Immutable<Placement> placement;
    bool stale = false;
};

class Placement {
public:
    /**
     * @brief creates a new placement instance, from the given update parameters
     * and the previous placement instance.
     *
     * Different placement implementations are created based on `updateParameters->mapMode`.
     * In Continuous map mode, `prevPlacement` must be provided.
     */
    static Mutable<Placement> create(std::shared_ptr<const UpdateParameters> updateParameters,
                                     std::optional<Immutable<Placement>> prevPlacement = std::nullopt);

    virtual ~Placement();
    virtual void placeLayers(const RenderLayerReferences&);
    void updateLayerBuckets(const RenderLayer&, const TransformState&, bool updateOpacities) const;
    virtual float symbolFadeChange(TimePoint now) const;
    virtual bool hasTransitions(TimePoint now) const;
    virtual bool transitionsEnabled() const;
    virtual void collectPlacedSymbolData(bool /*enable*/) {}
    virtual const std::vector<PlacedSymbolData>& getPlacedSymbolsData() const;

    const CollisionIndex& getCollisionIndex() const;
    TimePoint getCommitTime() const { return commitTime; }
    Duration getUpdatePeriod(float zoom) const;

    float zoomAdjustment(float zoom) const;
    const JointPlacement* getSymbolPlacement(const SymbolInstance&) const;

    const RetainedQueryData& getQueryData(uint32_t bucketInstanceId) const;

    // Public constructors are required for makeMutable(), shall not be called directly.
    Placement();
    Placement(std::shared_ptr<const UpdateParameters>, std::optional<Immutable<Placement>> prevPlacement);

protected:
    friend SymbolBucket;
    virtual void placeSymbolBucket(const BucketPlacementData&, std::set<uint32_t>& seenCrossTileIDs);
    JointPlacement placeSymbol(const SymbolInstance& symbolInstance, const PlacementContext&);
    void placeLayer(const RenderLayer&, std::set<uint32_t>&);
    virtual void commit();
    virtual void newSymbolPlaced(const SymbolInstance&,
                                 const PlacementContext&,
                                 const JointPlacement&,
                                 style::SymbolPlacementType,
                                 const std::vector<ProjectedCollisionBox>& /*textBoxes*/,
                                 const std::vector<ProjectedCollisionBox>& /*iconBoxes*/
    ) {}
    // Implementation specific hooks, which get called during a symbol bucket placement.
    virtual std::optional<CollisionBoundaries> getAvoidEdges(const SymbolBucket&, const mat4& /*posMatrix*/) {
        return std::nullopt;
    }
    SymbolInstanceReferences getSortedSymbols(const BucketPlacementData&, float pixelRatio);
    virtual bool canPlaceAtVariableAnchor(const CollisionBox&,
                                          style::TextVariableAnchorType,
                                          Point<float> /*shift*/,
                                          std::vector<style::TextVariableAnchorType>&,
                                          const mat4& /*posMatrix*/,
                                          float /*textPixelRatio*/
    ) {
        return true;
    }

    // Returns `true` if bucket vertices were updated; returns `false` otherwise.
    bool updateBucketDynamicAttributeData(SymbolBucket&, const TransformState&, const RenderTile& tile) const;
    void updateBucketOpacities(SymbolBucket&, const TransformState&, std::set<uint32_t>&) const;
    void markUsedJustification(SymbolBucket&,
                               style::TextVariableAnchorType,
                               const SymbolInstance&,
                               style::TextWritingModeType orientation) const;
    void markUsedOrientation(SymbolBucket&, style::TextWritingModeType, const SymbolInstance&) const;
    // With a keep-upright margin, syncs the reading directions of the line labels of one layer (see
    // LineLabelDirections) if this placement has not synced it yet, if one of its buckets has just
    // entered the cross tile index, or if its set of buckets has changed since; otherwise, and without a
    // margin, does nothing. Called whenever the opacities are updated, which can be on every frame: a
    // bucket of an overscaled tile with symbols out of view is registered again on every frame.
    void syncLineLabelDirections(const std::string& layerID, const LayerPlacementData&) const;
    const Placement* getPrevPlacement() const { return prevPlacement ? prevPlacement->get() : nullptr; }
    bool isTiltedView() const;

    std::shared_ptr<const UpdateParameters> updateParameters;
    CollisionIndex collisionIndex;

    style::TransitionOptions transitionOptions;

    TimePoint fadeStartTime;
    TimePoint commitTime;
    float placementZoom = 0.0f;
    float prevZoomAdjustment = 0.0f;

    std::unordered_map<uint32_t, JointPlacement> placements;
    std::unordered_map<uint32_t, JointOpacityState> opacities;
    std::unordered_map<uint32_t, VariableOffset> variableOffsets;
    std::unordered_map<uint32_t, style::TextWritingModeType> placedOrientations;
    // MapOptions::withKeepUprightMargin, in degrees; 0 (the default) outside MapMode::Continuous.
    float keepUprightMargin = 0.0f;
    // Synced in updateLayerBuckets(), next to the opacities it depends on; unused without a margin.
    mutable LineLabelDirections lineLabelDirections;
    // The buckets of each layer (bucket instance ids, sorted) when this placement last synced it.
    mutable std::unordered_map<std::string, std::vector<uint32_t>> lineLabelSyncedBuckets;

    std::unordered_map<uint32_t, RetainedQueryData> retainedQueryData;
    CollisionGroups collisionGroups;
    mutable std::optional<Immutable<Placement>> prevPlacement;
    bool showCollisionBoxes = false;

    // Cache being used by placeSymbol()
    std::vector<ProjectedCollisionBox> textBoxes;
    std::vector<ProjectedCollisionBox> iconBoxes;
    // Used for debug purposes.
    std::unordered_map<const CollisionFeature*, std::vector<ProjectedCollisionBox>> collisionCircles;
};

} // namespace mln
