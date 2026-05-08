#include <mbgl/tile/contour_tile.hpp>
#include <mbgl/tile/geojson_tile_data.hpp>
#include <mbgl/tile/raster_dem_tile.hpp>
#include <mbgl/util/constants.hpp>

#include <mapbox/feature.hpp>
#include <mapbox/geometry.hpp>

#include <cstdint>

namespace mbgl {

ContourTile::ContourTile(const OverscaledTileID& id_,
                         std::string sourceID_,
                         const TileParameters& parameters,
                         TileObserver* observer_)
    : GeometryTile(id_, std::move(sourceID_), parameters, observer_) {}

void ContourTile::populateFromDEM(const RasterDEMTile&) {
    // Spike: ignore the DEM data, emit a single hard-coded horizontal line
    // through the middle of the tile. Tile-local feature coordinates use
    // extent util::EXTENT (8192). Real marching-squares output replaces this
    // call body in Task 4.
    constexpr std::int16_t mid = static_cast<std::int16_t>(util::EXTENT / 2);

    mapbox::feature::feature<std::int16_t> feature;
    feature.geometry = mapbox::geometry::line_string<std::int16_t>{{0, mid},
                                                                   {static_cast<std::int16_t>(util::EXTENT), mid}};

    mapbox::feature::feature_collection<std::int16_t> features;
    features.push_back(std::move(feature));

    setData(std::make_unique<GeoJSONTileData>(std::move(features)));
}

} // namespace mbgl
