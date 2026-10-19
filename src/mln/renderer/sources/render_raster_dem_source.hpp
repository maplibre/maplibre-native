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
        const mat4& projMatrix) const override;

    std::vector<Feature> querySourceFeatures(const SourceQueryOptions&) const override;

    uint16_t getTileSize() const override;

    /// Lowest and highest elevation, in meters, of the DEM covering `id`: the tile's own once its
    /// data has loaded, otherwise the deepest loaded ancestor's, which contains it. A range is
    /// kept from the moment the tile's data arrives, also after the tile is dropped or evicted
    /// (up to maxElevationRanges), so the answer for an area changes only when DEM data arrives
    /// - not with which tiles the source happens to request or draw. nullopt when no loaded tile
    /// covers `id`.
    std::optional<Range<int32_t>> getElevationRange(const CanonicalTileID& id) const;

    /// Lowest and highest elevation, in meters, over all the ranges kept above; nullopt until a
    /// tile has loaded.
    const std::optional<Range<int32_t>>& getOverallElevationRange() const { return overallElevationRange; }

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

    void addElevationRange(const CanonicalTileID&, Range<int32_t>);
    /// Forget the ranges of tiles the pyramid no longer holds, in use or in its cache.
    void keepElevationRangesOfHeldTiles();

    std::unordered_map<CanonicalTileID, Range<int32_t>> elevationRanges;
    std::optional<Range<int32_t>> overallElevationRange;
    /// The tileset the ranges were decoded from.
    std::optional<Tileset> elevationRangesTileset;
    /// Some 50 bytes each. Past this many, the ranges of tiles the pyramid no longer holds are
    /// dropped. Far more than one view loads, so a still camera cannot cause a second drop.
    static constexpr std::size_t maxElevationRanges = 4096;
};

} // namespace mln
