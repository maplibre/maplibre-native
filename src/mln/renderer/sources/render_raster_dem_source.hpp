#pragma once

#include <mln/renderer/sources/render_tile_source.hpp>
#include <mln/style/sources/tile_source_impl.hpp>
#include <mln/util/range.hpp>

#include <optional>
#include <unordered_map>

namespace mln {

class RenderRasterDEMSource final : public RenderTileSetSource {
public:
    explicit RenderRasterDEMSource(Immutable<style::TileSource::Impl>, const TaggedScheduler&);

    std::unordered_map<std::string, std::vector<Feature>> queryRenderedFeatures(
        const ScreenLineString& geometry,
        const TransformState& transformState,
        const std::unordered_map<std::string, const RenderLayer*>& layers,
        const RenderedQueryOptions& options,
        const GlobalStateMap* globalState,
        const mat4& projMatrix) const override;

    std::vector<Feature> querySourceFeatures(const SourceQueryOptions&, const GlobalStateMap*) const override;

    uint16_t getTileSize() const override;

    /// Lowest and highest elevation, in meters, of the DEM covering `id`: the tile's own once its
    /// data has loaded, otherwise the deepest loaded ancestor's, which contains it. A range is
    /// kept from the moment the tile's data arrives, also after the tile is dropped or evicted
    /// (see maxElevationRangeEntries), so the answer for an area changes only when DEM data
    /// arrives - not with which tiles the source happens to request or draw. nullopt when no
    /// loaded tile covers `id`.
    std::optional<Range<int32_t>> getElevationRange(const CanonicalTileID& id) const;

    /// Lowest and highest elevation, in meters, of the loaded DEM nearest `id`, for a tile that
    /// no loaded tile covers: that of the loaded tiles inside `id` if there are any, otherwise of
    /// those inside its deepest ancestor that has any, so the whole of the loaded DEM only when
    /// nothing nearer has loaded. It contains the range of every loaded tile inside `id`, and
    /// like getElevationRange it changes only when DEM data arrives. nullopt until a tile has
    /// loaded.
    std::optional<Range<int32_t>> getNearbyElevationRange(const CanonicalTileID& id) const;

private:
    // RenderTileSetSource overrides
    void updateInternal(const Tileset&,
                        const std::vector<Immutable<style::LayerProperties>>&,
                        bool needsRendering,
                        bool needsRelayout,
                        const TileParameters&) override;
    const std::optional<Tileset>& getTileset() const override;
    void onTilesetChanged() override;

    const style::TileSource::Impl& impl() const;

    void onTileChanged(Tile&) override;

    void addElevationRange(const CanonicalTileID&, Range<int32_t>);
    /// Forget the ranges of tiles the pyramid no longer holds, in use or in its cache.
    void keepElevationRangesOfHeldTiles();

    /// The range of each tile whose data has arrived.
    std::unordered_map<CanonicalTileID, Range<int32_t>> elevationRanges;
    /// For every ancestor of a tile in elevationRanges, down to z0, the range over all of its
    /// descendants in there: what getNearbyElevationRange reads. They only ever widen, until the
    /// next drop below; the z0 entry spans every kept range of a tile below z0.
    std::unordered_map<CanonicalTileID, Range<int32_t>> descendantElevationRanges;
    /// Entries of the two maps together, some 60 bytes each, so about 0.5 MB at most. Past this
    /// many, the ranges of tiles the pyramid no longer holds are dropped, and the descendant
    /// ranges rebuilt from those it does. Far more than one view loads, ancestors included, so a
    /// still camera cannot cause a second drop.
    static constexpr std::size_t maxElevationRangeEntries = 8192;
};

} // namespace mln
