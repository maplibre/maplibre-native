#pragma once

#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/util/feature.hpp>

namespace mbgl {
namespace style {
class GeoJSONData;
} // namespace style

class TileParameters;

// NOTE: Any derived class must invalidate `weakFactory` in the destructor
class GeoJSONTile final : public GeometryTile {
public:
    GeoJSONTile(const OverscaledTileID&,
                std::string sourceID,
                const TileParameters&,
                std::shared_ptr<style::GeoJSONData>,
                TileObserver* observer = nullptr);

    void updateData(std::shared_ptr<style::GeoJSONData> data, bool needsRelayout, bool runSynchronously);

    void querySourceFeatures(std::vector<Feature>& result, const SourceQueryOptions&) override;

private:
    std::shared_ptr<style::GeoJSONData> data;
    mapbox::base::WeakPtrFactory<GeoJSONTile> weakFactory{this};
    // Do not add members here, see `WeakPtrFactory`
};

} // namespace mbgl
