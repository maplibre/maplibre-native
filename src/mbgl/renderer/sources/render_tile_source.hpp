#pragma once

#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/source_state.hpp>
#include <mbgl/renderer/tile_pyramid.hpp>
#include <mbgl/style/sources/vector_source_impl.hpp>
#include <mbgl/util/noncopyable.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/context.hpp>
#include <mbgl/tile/tile_diff.hpp>
#endif

namespace mbgl {

#if MLN_DRAWABLE_RENDERER
struct FrameTileDifference {
    FrameTileDifference(std::uint64_t prevFrame_, std::uint64_t curFrame_, TileDifference diff_)
        : prevFrame(prevFrame_),
          curFrame(curFrame_),
          diff(std::move(diff_)) {}

    std::uint64_t prevFrame;
    std::uint64_t curFrame;
    TileDifference diff;
};
#endif

/**
 * @brief Base class for render sources that provide render tiles.
 */
class RenderTileSource : public RenderSource {
public:
    ~RenderTileSource() override;

    bool isLoaded() const override;

    std::unique_ptr<RenderItem> createRenderItem() override;
    void prepare(const SourcePrepareParameters&) override;
    void updateFadingTiles() override;
    bool hasFadingTiles() const override;

    RenderTiles getRenderTiles() const override;
    RenderTiles getRenderTilesSortedByYPosition() const override;
    const Tile* getRenderedTile(const UnwrappedTileID&) const override;
    Immutable<std::vector<RenderTile>> getRawRenderTiles() const override { return renderTiles; }

#if MLN_DRAWABLE_RENDERER
    std::shared_ptr<FrameTileDifference> getRenderTileDiff() const override;
#endif

    std::unordered_map<std::string, std::vector<Feature>> queryRenderedFeatures(
        const ScreenLineString& geometry,
        const TransformState& transformState,
        const std::unordered_map<std::string, const RenderLayer*>& layers,
        const RenderedQueryOptions& options,
        const mat4& projMatrix) const override;

    std::vector<Feature> querySourceFeatures(const SourceQueryOptions&) const override;

    void setFeatureState(const std::optional<std::string>&, const std::string&, const FeatureState&) override;

    void getFeatureState(FeatureState& state, const std::optional<std::string>&, const std::string&) const override;

    void removeFeatureState(const std::optional<std::string>&,
                            const std::optional<std::string>&,
                            const std::optional<std::string>&) override;

    void setCacheEnabled(bool) override;
    void reduceMemoryUse() override;
    void dumpDebugLogs() const override;

protected:
    /// Must be called before updating the tile pyramid by derived classes overriding `update`
    void onTilePyramidWillUpdate();

    /// Must be called after updating the tile pyramid by derived classes overriding `update`
    void onTilePyramidUpdated();

    struct TilePyramidUpdateHelper;

    RenderTileSource(Immutable<style::Source::Impl>, const TaggedScheduler&);
    TilePyramid tilePyramid;

    // Note that while `renderTiles` is owned by a `shared_ptr`, its elements
    // contain bare references to `Tile` objects owned by `tilePyramid`.
    Immutable<std::vector<RenderTile>> renderTiles;
#if MLN_DRAWABLE_RENDERER
    bool renderTilesValid = false;
    std::uint64_t renderTilesPrevFrame = 0;
    std::uint64_t renderTilesCurFrame = 0;

    // The IDs of tiles in the previous state of `renderTiles`, and the differences with the current state
    std::vector<OverscaledTileID> previousRenderTiles;
    mutable std::shared_ptr<FrameTileDifference> renderTileDiff;
#endif

    // cached view of `renderTiles`, excluding those held for fading
    mutable RenderTiles filteredRenderTiles;
    // cached view of `renderTiles`, sorted by the Y tile coordinate
    mutable RenderTiles renderTilesSortedByY;

private:
    float bearing = 0.0F;
    SourceFeatureState featureState;
};

/// Eases the use of notification methods when updates are conditional
struct RenderTileSource::TilePyramidUpdateHelper : public util::noncopyable {
    TilePyramidUpdateHelper(RenderTileSource& src) : renderTileSource(src) {}

    void start() {
        if (!started) {
            started = true;
            renderTileSource.onTilePyramidWillUpdate();
        }
    }
    ~TilePyramidUpdateHelper() {
        if (started) {
            renderTileSource.onTilePyramidUpdated();
        }
    }

private:
    RenderTileSource& renderTileSource;
    bool started = false;
};

/**
 * @brief Base class for render sources that use tile sets.
 */
class RenderTileSetSource : public RenderTileSource {
protected:
    RenderTileSetSource(Immutable<style::Source::Impl>, const TaggedScheduler&);
    ~RenderTileSetSource() override;

    virtual void updateInternal(const Tileset&,
                                const std::vector<Immutable<style::LayerProperties>>&,
                                bool needsRendering,
                                bool needsRelayout,
                                const TileParameters&) = 0;
    // Returns tileset from the current impl.
    virtual const std::optional<Tileset>& getTileset() const = 0;

private:
    uint8_t getMaxZoom() const final;

    void update(Immutable<style::Source::Impl>,
                const std::vector<Immutable<style::LayerProperties>>&,
                bool needsRendering,
                bool needsRelayout,
                const TileParameters&) final;

    std::optional<Tileset> cachedTileset;
};

class TileSourceRenderItem : public RenderItem {
public:
    TileSourceRenderItem(Immutable<std::vector<RenderTile>> renderTiles_, std::string name_)
        : renderTiles(std::move(renderTiles_)),
          name(std::move(name_)) {}

private:
    void upload(gfx::UploadPass&) const override;
    void render(PaintParameters&) const override;
    bool hasRenderPass(RenderPass) const override { return false; }
    const std::string& getName() const override { return name; }

#if MLN_DRAWABLE_RENDERER
    void updateDebugDrawables(DebugLayerGroupMap&, PaintParameters&) const override;
#endif

    Immutable<std::vector<RenderTile>> renderTiles;
    std::string name;
};

} // namespace mbgl
