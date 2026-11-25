#pragma once

#include <mln/tile/tile_id.hpp>
#include <mln/tile/tile_necessity.hpp>
#include <mln/util/range.hpp>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <type_traits>
#include <unordered_set>

namespace mln {
namespace algorithm {

/// Zoom limits of the cached tile fallback in `updateRenderables`.
struct CachedTileFallbackLimits {
    uint8_t maxParentZoomDelta;
    uint8_t maxChildZoomDelta;
};

namespace detail {

/// Draws the renderable cached descendants of `tileID` down to `depthLeft`
/// levels. Returns whether they cover `tileID` completely.
template <typename GetCachedTileFn, typename RetainTileFn, typename RenderTileFn>
bool renderCachedChildren(GetCachedTileFn& getCachedTile,
                          RetainTileFn& retainTile,
                          RenderTileFn& renderTile,
                          const OverscaledTileID& tileID,
                          const Range<uint8_t>& zoomRange,
                          uint8_t depthLeft) {
    if (depthLeft == 0 || tileID.overscaledZ == std::numeric_limits<uint8_t>::max()) {
        return false;
    }
    const auto childZ = static_cast<uint8_t>(tileID.overscaledZ + 1);
    auto visit = [&](const OverscaledTileID& childID) -> bool {
        if (auto* tile = getCachedTile(childID)) {
            retainTile(*tile, TileNecessity::Optional);
            renderTile(childID.toUnwrapped(), *tile);
            return true;
        }
        return renderCachedChildren(
            getCachedTile, retainTile, renderTile, childID, zoomRange, static_cast<uint8_t>(depthLeft - 1));
    };
    if (childZ > zoomRange.max) {
        // Past the source max zoom, the only child is the overscaled tile.
        return visit(OverscaledTileID(childZ, tileID.wrap, tileID.canonical));
    }
    bool covered = true;
    for (const auto& child : tileID.canonical.children()) {
        covered &= visit(OverscaledTileID(childZ, tileID.wrap, child));
    }
    return covered;
}

/// Cached tile fallback for an ideal tile that is not renderable. Returns
/// whether any tile was drawn.
template <typename GetCachedTileFn, typename RetainTileFn, typename RenderTileFn>
bool renderCachedFallback(GetCachedTileFn& getCachedTile,
                          RetainTileFn& retainTile,
                          RenderTileFn& renderTile,
                          const OverscaledTileID& idealTileID,
                          const Range<uint8_t>& zoomRange,
                          const std::optional<uint8_t>& maxParentOverscaleFactor,
                          const CachedTileFallbackLimits& limits) {
    bool rendered = false;
    auto renderAndMark = [&](const UnwrappedTileID& renderID, auto& tile) {
        renderTile(renderID, tile);
        rendered = true;
    };
    // Children first: they are drawn above a parent, so a parent is only
    // needed where they leave gaps.
    if (renderCachedChildren(
            getCachedTile, retainTile, renderAndMark, idealTileID, zoomRange, limits.maxChildZoomDelta)) {
        return true;
    }

    uint8_t maxParentDelta = limits.maxParentZoomDelta;
    if (maxParentOverscaleFactor) {
        maxParentDelta = std::min(maxParentDelta, *maxParentOverscaleFactor);
    }
    for (int32_t delta = 1; delta <= maxParentDelta; ++delta) {
        const int32_t parentZ = idealTileID.overscaledZ - delta;
        if (parentZ < zoomRange.min) {
            break;
        }
        const auto parentID = idealTileID.scaledTo(static_cast<uint8_t>(parentZ));
        if (auto* tile = getCachedTile(parentID)) {
            retainTile(*tile, TileNecessity::Optional);
            renderTile(parentID.toUnwrapped(), *tile);
            rendered = true;
            break;
        }
    }
    return rendered;
}

} // namespace detail

/// Selects the tiles to render for the given ideal tiles.
///
/// `getCachedTile`, when given, enables the cached tile fallback: it returns a
/// renderable tile for the ID that can be drawn without loading it (one already
/// held or one in the tile cache), or `nullptr`. When an ideal tile is not
/// renderable and the regular child and parent search does not find a parent,
/// cached children up to `maxChildZoomDelta` levels below are drawn and, if
/// they do not cover the ideal tile, the nearest cached parent up to
/// `maxParentZoomDelta` levels above is drawn under them.
template <typename GetTileFn,
          typename CreateTileFn,
          typename RetainTileFn,
          typename RenderTileFn,
          typename IdealTileIDs,
          typename PrefetchedTileMap,
          typename GetCachedTileFn = std::nullptr_t>
void updateRenderables(GetTileFn getTile,
                       CreateTileFn createTile,
                       RetainTileFn retainTile,
                       RenderTileFn renderTile,
                       const IdealTileIDs& idealTileIDs,
                       const PrefetchedTileMap& prefetchedTiles,
                       const Range<uint8_t>& zoomRange,
                       const std::optional<uint8_t>& maxParentOverscaleFactor = std::nullopt,
                       GetCachedTileFn getCachedTile = nullptr,
                       const CachedTileFallbackLimits& fallbackLimits = {}) {
    std::unordered_set<OverscaledTileID> checked;
    bool covered = false;
    bool parentOrChildTileFound = false;
    bool parentTileFound = false;
    int32_t overscaledZ = 0;

    // for (all in the set of ideal tiles of the source) {
    for (const auto& idealDataTileID : idealTileIDs) {
        assert(idealDataTileID.canonical.z >= zoomRange.min);
        assert(idealDataTileID.canonical.z <= zoomRange.max);
        assert(idealDataTileID.overscaledZ >= idealDataTileID.canonical.z);

        const UnwrappedTileID idealRenderTileID = idealDataTileID.toUnwrapped();
        auto tile = getTile(idealDataTileID);
        if (!tile) {
            tile = createTile(idealDataTileID);
            // For source types where TileJSON.bounds is set, tiles outside the
            // bounds are not created
            if (tile == nullptr) {
                continue;
            }
        }

        // if (source has the tile and bucket is loaded) {
        if (tile->isRenderable()) {
            retainTile(*tile, TileNecessity::Required);
            renderTile(idealRenderTileID, *tile);
        } else {
            // We are now attempting to load child and parent tiles.
            bool parentHasTriedOptional = tile->hasTriedCache();
            bool parentIsLoaded = tile->isLoaded();

            // The tile isn't loaded yet, but retain it anyway because it's an ideal tile.
            retainTile(*tile, TileNecessity::Required);
            covered = true;
            parentOrChildTileFound = false;
            parentTileFound = false;
            overscaledZ = idealDataTileID.overscaledZ + 1;
            if (std::cmp_greater(overscaledZ, zoomRange.max)) {
                // We're looking for an overzoomed child tile.
                const auto childDataTileID = idealDataTileID.scaledTo(overscaledZ);
                tile = getTile(childDataTileID);
                if (tile && tile->isRenderable()) {
                    retainTile(*tile, TileNecessity::Optional);
                    renderTile(idealRenderTileID, *tile);
                    parentOrChildTileFound = true;
                } else {
                    covered = false;
                }
            } else {
                // Check all four actual child tiles.
                for (const auto& childTileID : idealDataTileID.canonical.children()) {
                    const OverscaledTileID childDataTileID(overscaledZ, idealRenderTileID.wrap, childTileID);
                    tile = getTile(childDataTileID);
                    if (tile && tile->isRenderable()) {
                        retainTile(*tile, TileNecessity::Optional);
                        renderTile(childDataTileID.toUnwrapped(), *tile);
                        parentOrChildTileFound = true;
                    } else {
                        // At least one child tile doesn't exist, so we are
                        // going to look for parents as well.
                        covered = false;
                    }
                }
            }

            if (!covered) {
                // We couldn't find child tiles that entirely cover the ideal tile.
                for (overscaledZ = idealDataTileID.overscaledZ - 1; std::cmp_greater_equal(overscaledZ, zoomRange.min);
                     --overscaledZ) {
                    const auto parentDataTileID = idealDataTileID.scaledTo(overscaledZ);

                    // Request / render parent tile only if it's overscale
                    // factor is less than defined maximum.
                    if (maxParentOverscaleFactor &&
                        (idealDataTileID.overscaledZ - overscaledZ) > *maxParentOverscaleFactor) {
                        break;
                    }

                    if (checked.find(parentDataTileID) != checked.end()) {
                        // Break parent tile ascent, this route has been checked
                        // by another child tile before.
                        break;
                    } else {
                        checked.emplace(parentDataTileID);
                    }

                    tile = getTile(parentDataTileID);
                    if (!tile && (parentHasTriedOptional || parentIsLoaded)) {
                        tile = createTile(parentDataTileID);
                    }

                    if (tile) {
                        if (!parentIsLoaded) {
                            // We haven't completed loading the child, so we
                            // only do an optional (cache) request in an attempt
                            // to quickly load data that we can show.
                            retainTile(*tile, TileNecessity::Optional);
                        } else {
                            // Now that we've checked the child and know for
                            // sure that we can't load it, we attempt to load
                            // the parent from the network.
                            retainTile(*tile, TileNecessity::Required);
                        }

                        // Save the current values, since they're the parent of
                        // the next iteration of the parent tile ascent loop.
                        parentHasTriedOptional = tile->hasTriedCache();
                        parentIsLoaded = tile->isLoaded();

                        if (tile->isRenderable()) {
                            renderTile(parentDataTileID.toUnwrapped(), *tile);
                            parentOrChildTileFound = true;
                            parentTileFound = true;
                            // Break parent tile ascent, since we found one.
                            break;
                        }
                    }
                }

                if constexpr (!std::is_same_v<GetCachedTileFn, std::nullptr_t>) {
                    if (!parentTileFound) {
                        parentOrChildTileFound |= detail::renderCachedFallback(getCachedTile,
                                                                               retainTile,
                                                                               renderTile,
                                                                               idealDataTileID,
                                                                               zoomRange,
                                                                               maxParentOverscaleFactor,
                                                                               fallbackLimits);
                    }
                }

                if (!parentOrChildTileFound) {
                    // Reuse prefetched tiles in order to avoid empty screen
                    for (auto& prefetchedTileEntry : prefetchedTiles) {
                        const auto& prefetchedDataTileID = prefetchedTileEntry.first;
                        const UnwrappedTileID prefetchedRenderTileID = prefetchedDataTileID.toUnwrapped();
                        auto* prefetchedTile = prefetchedTileEntry.second.get();
                        if (prefetchedTile->isRenderable() && prefetchedDataTileID.canonical.z <= zoomRange.max &&
                            prefetchedDataTileID.isChildOf(idealDataTileID)) {
                            retainTile(*prefetchedTile, TileNecessity::Optional);
                            renderTile(prefetchedRenderTileID, *prefetchedTile);
                        }
                    }
                }
            }
        }
    }
}

} // namespace algorithm
} // namespace mln
