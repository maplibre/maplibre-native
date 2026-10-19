#include <mln/renderer/dem_elevation_provider.hpp>

#include <mln/renderer/sources/render_raster_dem_source.hpp>

namespace mln {

DEMElevationProvider::DEMElevationProvider(const RenderSource* demSource_, double exaggeration_)
    : demSource(demSource_ && demSource_->baseImpl->type == style::SourceType::RasterDEM
                    ? static_cast<const RenderRasterDEMSource*>(demSource_)
                    : nullptr),
      exaggeration(exaggeration_) {}

std::optional<Range<double>> DEMElevationProvider::getTileElevationRange(const CanonicalTileID& id) const {
    if (!demSource) {
        return std::nullopt;
    }

    // The tile's own DEM, or failing that the deepest loaded ancestor's, or failing that the
    // range of the loaded DEM nearest the tile (see the header).
    auto range = demSource->getElevationRange(id);
    if (!range) {
        range = demSource->getNearbyElevationRange(id);
    }
    if (!range) {
        return std::nullopt;
    }

    // Exaggeration is applied to the mesh in the terrain vertex shader, so the bounds
    // have to carry it too, or an exaggerated peak would still be culled.
    return Range<double>{range->min * exaggeration, range->max * exaggeration};
}

} // namespace mln
