#pragma once

#include <mbgl/renderer/sources/render_tile_source.hpp>
#include <mbgl/style/sources/tile_source_impl.hpp>
#include <mbgl/util/listener_set.hpp>

namespace mbgl {

class RasterDEMTile;

class RenderRasterDEMSource final : public RenderTileSetSource {
public:
    explicit RenderRasterDEMSource(Immutable<style::TileSource::Impl>, const TaggedScheduler&);

    std::unordered_map<std::string, std::vector<Feature>> queryRenderedFeatures(
        const ScreenLineString& geometry,
        const TransformState& transformState,
        const std::unordered_map<std::string, const RenderLayer*>& layers,
        const RenderedQueryOptions& options,
        const mat4& projMatrix) const override;

    std::vector<Feature> querySourceFeatures(const SourceQueryOptions&) const override;

    // Tile-load listener API. Cross-source consumers (e.g. ContourSource)
    // register a listener and are notified every time a DEM tile finishes
    // parsing with `DEMData` ready. `addTileLoadListener` also replays the
    // currently-renderable tiles synchronously so a late-registering
    // consumer doesn't miss tiles already loaded before it appeared.
    using TileLoadListener = std::function<void(const RasterDEMTile&)>;
    using ListenerHandle = ListenerSet<const RasterDEMTile&>::Handle;
    [[nodiscard]] ListenerHandle addTileLoadListener(TileLoadListener);
    void removeTileLoadListener(ListenerHandle);

    // Sync lookup: returns the renderable DEM tile at the given coord, or
    // nullptr if absent / not yet parsed. Used by cross-source consumers
    // (contour source) to populate newly-created derived tiles from
    // already-loaded DEM data, closing the race between consumer-pyramid
    // updates and upstream tile arrivals.
    const RasterDEMTile* getRenderableTile(const OverscaledTileID&) const;

private:
    // RenderTileSetSource overrides
    void updateInternal(const Tileset&,
                        const std::vector<Immutable<style::LayerProperties>>&,
                        bool needsRendering,
                        bool needsRelayout,
                        const TileParameters&) override;
    const std::optional<Tileset>& getTileset() const override;

    const style::TileSource::Impl& impl() const;

    void onTileChanged(Tile&) override;

    ListenerSet<const RasterDEMTile&> tileLoadListeners;
};

} // namespace mbgl
