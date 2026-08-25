#pragma once

#include <mln/map/mode.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/style/image_impl.hpp>
#include <mln/text/glyph.hpp>
#include <mln/text/glyph_manager.hpp>
#include <mln/actor/optional_actor_ref.hpp>
#include <mln/util/immutable.hpp>
#include <mln/style/layer_properties.hpp>
#include <mln/geometry/feature_index.hpp>
#include <mln/renderer/bucket.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/containers.hpp>

#include <atomic>
#include <memory>

namespace mln {

class GeometryTile;
class GeometryTileData;
class Layout;

namespace style {
class Layer;
} // namespace style

namespace gfx {
class DynamicTextureAtlas;
using DynamicTextureAtlasPtr = std::shared_ptr<gfx::DynamicTextureAtlas>;
} // namespace gfx

class GeometryTileWorker {
public:
    GeometryTileWorker(OptionalActorRef<GeometryTileWorker> self,
                       OptionalActorRef<GeometryTile> parent,
                       const TaggedScheduler& scheduler_,
                       OverscaledTileID,
                       std::string,
                       const std::atomic<bool>&,
                       MapMode,
                       float pixelRatio,
                       bool showCollisionBoxes_,
                       gfx::DynamicTextureAtlasPtr,
                       std::shared_ptr<FontFaces> fontFaces,
                       TileObserver* observer);
    ~GeometryTileWorker();

    void setObserver(TileObserver* observer);

    void setLayers(std::vector<Immutable<style::LayerProperties>>,
                   std::set<std::string> availableImages,
                   uint64_t correlationID);
    void setData(std::unique_ptr<const GeometryTileData>,
                 std::set<std::string> availableImages,
                 uint64_t correlationID);
    void reset(uint64_t correlationID_);
    void setShowCollisionBoxes(bool showCollisionBoxes_, uint64_t correlationID_);

    void onGlyphsAvailable(GlyphMap glyphs, HBShapeResults requests);

    void onImagesAvailable(ImageMap newIconMap,
                           ImageMap newPatternMap,
                           ImageVersionMap versionMap,
                           uint64_t imageCorrelationID);

private:
    void coalesced();
    void parse();
    void finalizeLayout();

    void coalesce();

    void requestNewGlyphs(const GlyphDependencies&);
    void requestNewImages(const ImageDependencies&);

    void symbolDependenciesChanged();
    bool hasPendingDependencies() const;
    bool hasPendingParseResult() const;

    void checkPatternLayout(std::unique_ptr<Layout> layout);

    OptionalActorRef<GeometryTileWorker> self;
    OptionalActorRef<GeometryTile> parent;
    TaggedScheduler scheduler;

    const OverscaledTileID id;
    const std::string sourceID;
    const std::atomic<bool>& obsolete;
    const MapMode mode;
    const float pixelRatio;

    std::unique_ptr<FeatureIndex> featureIndex;
    mln::unordered_map<std::string, LayerRenderData> renderData;

    enum State {
        Idle,
        Coalescing,
        NeedsParse,
        NeedsSymbolLayout
    };

    State state = Idle;
    uint64_t correlationID = 0;
    uint64_t imageCorrelationID = 0;

    // Outer std::optional indicates whether we've received it or not.
    std::optional<std::vector<Immutable<style::LayerProperties>>> layers;
    std::optional<std::unique_ptr<const GeometryTileData>> data;

    std::vector<std::unique_ptr<Layout>> layouts;

    GlyphDependencies pendingGlyphDependencies;
    ImageDependencies pendingImageDependencies;
    GlyphMap glyphMap;
    ImageMap iconMap;
    ImageMap patternMap;
    ImageVersionMap versionMap;
    std::set<std::string> availableImages;

    bool showCollisionBoxes;
    bool firstLoad = true;

    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;

    std::shared_ptr<FontFaces> fontFaces;

    TileObserver* observer;
};

} // namespace mln
