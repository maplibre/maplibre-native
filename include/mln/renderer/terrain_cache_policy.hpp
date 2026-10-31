#pragma once

#include <mln/tile/tile_id.hpp>
#include <mln/util/hash.hpp>

#include <cstdint>
#include <optional>
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

/// What one draped layer group would render into a drape target, gathered tile by tile from the
/// tiles the group draws on and the tiles its layer has no data on. Tiles that do not overlap the
/// target are ignored.
///
/// `contentHash` identifies the content, so the drape re-renders when it changes. It is keyed on
/// the tiles and the data drawn on them, NOT on drawable instances: those are rebuilt with fresh
/// ids on every bucket update and fade transition, and a drawable-id hash churned every frame
/// while panning and re-rendered every drape (~4x overdraw). Summed, so the order tiles are added
/// in does not matter. Matches maplibre-gl-js, which keys its terrain render cache on tile coverage.
///
/// `fallback` tells drapeFallbackStrictlyWorse whether the content got worse. A tile the layer has
/// no data on is content, empty content: a GeoJSON source updated to have no features there must
/// clear the drape too. A tile neither drawn nor empty - still loading, or with its drawables
/// deferred by the tile build budget - is missing, and meanwhile the drape keeps its bake.
class DrapeGroupCoverage {
public:
    DrapeGroupCoverage(const UnwrappedTileID& target_, int32_t layerIndex_)
        : target(target_),
          layerIndex(layerIndex_) {}

    /// A tile the group draws on, once per drawable. `dataID` identifies the data drawn there (the
    /// source bucket's id, 0 when unknown), so new data under the same tile - a GeoJSON source
    /// updated with setGeoJson - is new content.
    void addDrawn(const UnwrappedTileID& tile, std::size_t dataID) { add(tile, /*drawn=*/true, dataID); }

    /// A tile the group's layer covers but has no data on
    void addEmpty(const UnwrappedTileID& tile) { add(tile, /*drawn=*/false, 0); }

    std::size_t contentHash() const { return hash; }

    DrapeGroupFallback fallback() const {
        // Only the coarsest standalone fallback counts as lost detail; when an exact or deeper
        // tile is present the ancestor is clipped away by the tile masks and costs nothing.
        if (exactOrDescendant) {
            return {layerIndex, 0};
        }
        if (bestAncestorZoom) {
            return {layerIndex, static_cast<int16_t>(target.canonical.z - *bestAncestorZoom)};
        }
        return {layerIndex, -1};
    }

private:
    void add(const UnwrappedTileID& tile, bool drawn, std::size_t dataID) {
        if (tile == target || tile.isChildOf(target)) {
            exactOrDescendant = true;
        } else if (target.isChildOf(tile)) {
            if (!bestAncestorZoom || tile.canonical.z > *bestAncestorZoom) {
                bestAncestorZoom = tile.canonical.z;
            }
        } else {
            return;
        }
        std::size_t tileHash = 0;
        util::hash_combine(tileHash, tile.wrap);
        util::hash_combine(tileHash, tile.canonical.z);
        util::hash_combine(tileHash, tile.canonical.x);
        util::hash_combine(tileHash, tile.canonical.y);
        util::hash_combine(tileHash, drawn);
        util::hash_combine(tileHash, dataID);
        hash += tileHash;
    }

    UnwrappedTileID target;
    int32_t layerIndex;
    bool exactOrDescendant = false;
    std::optional<uint8_t> bestAncestorZoom;
    std::size_t hash = 0;
};

} // namespace terrain
} // namespace mln
