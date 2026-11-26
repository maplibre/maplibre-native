#pragma once

#include <mln/tile/geometry_tile.hpp>
#include <mln/util/callback_scope.hpp>
#include <mln/util/feature.hpp>

namespace mln {
namespace style {
class GeoJSONData;
} // namespace style

class TileParameters;

class GeoJSONTile final : public GeometryTile {
public:
    GeoJSONTile(const OverscaledTileID&,
                std::string sourceID,
                const TileParameters&,
                std::shared_ptr<style::GeoJSONData>,
                TileObserver* observer = nullptr);
    ~GeoJSONTile() override;

    void updateData(std::shared_ptr<style::GeoJSONData> data, bool needsRelayout, bool runSynchronously);

    void querySourceFeatures(std::vector<Feature>& result,
                             const SourceQueryOptions&,
                             const GlobalStateMap* globalState) override;

private:
    std::shared_ptr<style::GeoJSONData> data;
    util::CallbackScope callbacks;
};

} // namespace mln
