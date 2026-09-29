#pragma once

#include <cstdint>
#include <vector>

namespace mln {
namespace terrain {

/// The DEM a terrain mesh drawable was built on.
struct DEMBinding {
    /// 0 = flat placeholder, 1 = closest loaded ancestor DEM, 2 = the tile's own DEM
    uint8_t tier = 0;
    /// Zoom of the bound DEM tile (-1 for the placeholder)
    int8_t demZoom = -1;
};

/// Whether a terrain drawable built on `current` should be rebuilt now that `available` could be
/// bound instead. A better tier always wins. Within the ancestor tier a deeper ancestor wins as
/// well: a fresh wide view often builds its tiles on whatever coarse ancestor is loaded first (z6
/// under a z13 tile samples 1/128th of that DEM, which renders flat), and the deeper ancestors that
/// load moments later must replace it. maplibre-gl-js resolves the covering DEM tile every frame
/// (Terrain.getTerrainData -> TileManager.getSourceTile(tileID, true)), so it never keeps a
/// shallower one than it has.
inline bool shouldRebind(const DEMBinding& current, const DEMBinding& available) {
    if (available.tier != current.tier) {
        return available.tier > current.tier;
    }
    return available.tier == 1 && available.demZoom > current.demZoom;
}

/// How one draped layer group covers a drape target.
struct DrapeGroupFallback {
    int32_t layerIndex = 0;
    /// -1 = no usable tile, 0 = exact or deeper tiles, n > 0 = only an ancestor n zoom levels up
    int16_t deficit = -1;
};

/// Whether the content a drape target would render now (`now`) is strictly worse than what it
/// baked (`baked`): no layer group improved and at least one lost its content or fell back to a
/// coarser ancestor. The drape cache keeps its baked texture in that case, so a tile briefly
/// dropping out of the render set (eviction, reload) does not flash the drape empty.
///
/// Compared per layer group, not as a sum over groups: a sum lets one group's regression outweigh
/// another group's improvement, and the improvement - typically hillshade arriving from its real
/// DEM tile - is then discarded for as long as the target lives. Group sets that differ (style
/// change) are not comparable and never count as worse.
inline bool drapeFallbackStrictlyWorse(const std::vector<DrapeGroupFallback>& now,
                                       const std::vector<DrapeGroupFallback>& baked) {
    if (now.size() != baked.size()) {
        return false;
    }
    bool worse = false;
    for (std::size_t i = 0; i < now.size(); ++i) {
        const auto& n = now[i];
        const auto& b = baked[i];
        if (n.layerIndex != b.layerIndex) {
            return false;
        }
        if (n.deficit == b.deficit) {
            continue;
        }
        const bool improved = (b.deficit < 0) || (n.deficit >= 0 && n.deficit < b.deficit);
        if (improved) {
            return false;
        }
        worse = true;
    }
    return worse;
}

} // namespace terrain
} // namespace mln
