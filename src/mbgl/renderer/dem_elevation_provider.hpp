#pragma once

#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/range.hpp>
#include <mbgl/util/tile_cover.hpp>

#include <optional>

namespace mbgl {

class RenderSource;

/// Answers "how high is the terrain here?" for util::tileCover, from the DEM source's
/// currently loaded tiles, so that sources are asked for the tiles the terrain mesh
/// actually needs (see util::TileElevationProvider).
///
/// A tile's own DEM is used when loaded, otherwise the nearest loaded ancestor's range,
/// which contains it. The answer only has to be conservative: too large an elevation
/// range loads a tile that turns out not to be visible, too small drops one that is.
class DEMElevationProvider final : public util::TileElevationProvider {
public:
    /// `demSource` may be null (no terrain, or its source not yet resolved), in which
    /// case every query reports unknown and the cover stays flat.
    explicit DEMElevationProvider(const RenderSource* demSource, double exaggeration);

    std::optional<Range<double>> getTileElevationRange(const CanonicalTileID&) const override;

private:
    const RenderSource* demSource;
    double exaggeration;
};

} // namespace mbgl
