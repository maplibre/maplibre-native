#pragma once

#include <mln/tile/tile_id.hpp>
#include <mln/util/range.hpp>
#include <mln/util/tile_cover.hpp>

#include <optional>

namespace mln {

class RenderSource;
class RenderRasterDEMSource;

/// Answers "how high is the terrain here?" for util::tileCover, from the DEM data the DEM source
/// has loaded, so that sources are asked for the tiles the terrain mesh actually needs (see
/// util::TileElevationProvider).
///
/// A tile's own DEM is used once it has loaded, otherwise the deepest loaded ancestor's range,
/// which contains it. The answer only has to be conservative: too large an elevation range
/// loads a tile that turns out not to be visible, too small drops one that is. It must also
/// not depend on which tiles are currently requested or drawn, because the cover decides that:
/// see RenderRasterDEMSource::getElevationRange.
///
/// For a tile with *no* loaded DEM covering it, it reports the range of the loaded terrain
/// nearest to it rather than "unknown": that of the loaded tiles inside it, or failing that of
/// those inside its deepest ancestor that has any (see
/// RenderRasterDEMSource::getNearbyElevationRange). This is what lets the one-ring cover
/// dilation in RenderTerrain::computeMeshCover re-test a not-yet-loaded frontier neighbour
/// against the elevation-extended frustum: an unknown tile is assumed about as tall (or, for
/// bathymetry, as deep) as the terrain around it, so the tile that belongs in view is kept and
/// requested instead of assumed flat and dropped. It contains the range of every loaded tile
/// inside the tile, so it also never culls a large tile of the cover's traversal (above the
/// DEM's zoom levels) whose loaded descendants are in view. And it stays local: terrain loaded
/// far away, such as the mountains the camera came from, does not raise tiles next to low
/// ground that has loaded. nullopt only when nothing at all has loaded (cover stays flat,
/// exactly as before).
class DEMElevationProvider final : public util::TileElevationProvider {
public:
    /// `demSource` may be null (no terrain, or its source not yet resolved), in which
    /// case every query reports unknown and the cover stays flat.
    explicit DEMElevationProvider(const RenderSource* demSource, double exaggeration);

    std::optional<Range<double>> getTileElevationRange(const CanonicalTileID&) const override;

private:
    const RenderRasterDEMSource* demSource;
    double exaggeration;
};

} // namespace mln
