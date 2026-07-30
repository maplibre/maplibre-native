#include <mbgl/renderer/render_terrain.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/renderer/dem_elevation_provider.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/terrain_layer_tweaker.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/geometry/dem_data.hpp>
#include <mbgl/util/tile_cover.hpp>
#include <mbgl/tile/raster_dem_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/terrain_layer_ubo.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/monotonic_timer.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/map/camera.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <iomanip>
#include <sstream>
#include <unordered_set>

namespace mbgl {

namespace {

// Scale and x/y offset mapping a child tile's local space into the (possibly
// ancestor) DEM tile that covers it: a child dz levels deeper occupies the
// 1/scale sub-square at (dx, dy) of the ancestor.
struct DEMSubTileOffset {
    float scale;
    float dx;
    float dy;
};

DEMSubTileOffset demSubTileOffset(const CanonicalTileID& child, const CanonicalTileID& ancestor) {
    const int dz = child.z - ancestor.z;
    return {static_cast<float>(1u << dz),
            static_cast<float>(child.x - (ancestor.x << dz)),
            static_cast<float>(child.y - (ancestor.y << dz))};
}

} // namespace

RenderTerrain::RenderTerrain(Immutable<style::Terrain::Impl> impl_)
    : impl(std::move(impl_)) {}

RenderTerrain::~RenderTerrain() = default;

std::set<UnwrappedTileID> RenderTerrain::computeMeshCover(
    const TransformState& state, const std::shared_ptr<UpdateParameters>& updateParameters) const {
    std::set<UnwrappedTileID> out;
    if (!demSource) {
        return out;
    }

    // Elevation-aware visibility: terrain leaning towards the camera occupies
    // screen space its flat footprint does not, so the cover is tested against the
    // DEM height rather than the z=0 plane (as util::tileCover / gl-js do).
    DEMElevationProvider elevationProvider(demSource, getExaggeration());

    // Cover at the DEM source's own tile size so the mesh zoom matches the DEM's native
    // granularity. A 256px DEM covers one zoom level deeper than a 512px one; meshing
    // coarser than the DEM undersamples it - the fixed 128x128 mesh aliases the relief
    // into waves (seen on the 256px debug-tiles "ruffles" DEM, which was flat before the
    // ideal-cover rework derived the mesh from the DEM's own render set). demDim is the
    // decoded DEM's tile size (DEMData::dim, exact - border is separate); fall back to 512
    // (gl-js's terrain-cover convention) until the first DEM tile is decoded. The cover is
    // still the elevation-aware ideal cover from the view, not the DEM's loaded tile set.
    const uint16_t terrainCoverTileSize = demDim > 0 ? static_cast<uint16_t>(demDim) : 512;
    const Range<uint8_t> zoomRange{0, demSource->getMaxZoom()};

    // LOD parameters from the frame drive the same near-high/far-low zoom
    // selection every other source uses, so the near field drapes at a higher
    // zoom (smaller ground area per 1024 target = sharper draped content).
    util::TileCoverParameters coverParams{.transformState = state, .elevationProvider = &elevationProvider};
    double zoomShift = 0.0;
    if (updateParameters) {
        coverParams.tileLodMinRadius = updateParameters->tileLodMinRadius;
        coverParams.tileLodScale = updateParameters->tileLodScale;
        coverParams.tileLodPitchThreshold = updateParameters->tileLodPitchThreshold;
        coverParams.tileLodMode = updateParameters->tileLodMode;
        zoomShift = updateParameters->tileLodZoomShift;
    }

    const double zoom = util::clamp<double>(state.getZoom() + zoomShift, state.getMinZoom(), state.getMaxZoom());
    const int32_t overscaledZoom = util::coveringZoomLevel(zoom, style::SourceType::RasterDEM, terrainCoverTileSize);
    if (overscaledZoom < static_cast<int32_t>(zoomRange.min)) {
        return out;
    }
    const int32_t idealZoom = std::min<int32_t>(zoomRange.max, overscaledZoom);
    for (const auto& id : util::tileCover(
             coverParams, static_cast<uint8_t>(idealZoom), zoomRange, static_cast<uint8_t>(overscaledZoom))) {
        out.insert(id.toUnwrapped());
    }

    // One-ring cover dilation. tileCover is a top-down DFS that only descends into tiles
    // whose (sea-level) ancestor intersects the frustum, so a frontier tile whose terrain
    // rises into view but whose flat ancestor was culled is never even visited - it stays
    // out of the cover, and the terrain draws a skirt with nothing behind it, until a
    // camera nudge shifts the frustum and pulls it in. Add every 8-neighbour of the cover,
    // then keep only those whose elevation-extended bounds still intersect the frustum
    // (frustumCull, using the DEM provider's conservative fallback range for the
    // not-yet-loaded neighbours). The elevation sign decides which neighbours survive, so
    // this is correct for terrain above OR below sea level (bathymetry) without hardcoding
    // a direction; the frustum trim stops it from tripling the cover like a blind ring
    // would. Drop the frustumCull line to fall back to a pure uniform 1-ring dilation.
    std::set<UnwrappedTileID> dilated = out;
    for (const auto& id : out) {
        const int32_t numTiles = 1 << id.canonical.z;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;
                const int ny = static_cast<int>(id.canonical.y) + dy;
                if (ny < 0 || ny >= numTiles) continue; // nothing past the poles
                int nx = static_cast<int>(id.canonical.x) + dx;
                int nwrap = id.wrap;
                if (nx < 0) {
                    nx += numTiles;
                    --nwrap;
                } else if (nx >= numTiles) {
                    nx -= numTiles;
                    ++nwrap;
                }
                dilated.emplace(static_cast<int16_t>(nwrap),
                                CanonicalTileID(id.canonical.z, static_cast<uint32_t>(nx), static_cast<uint32_t>(ny)));
            }
        }
    }
    if (dilated.size() != out.size()) {
        out = util::frustumCull(coverParams, dilated);
    }
    return out;
}

void RenderTerrain::update(RenderOrchestrator& orchestrator,
                           gfx::ShaderRegistry& shaders,
                           gfx::Context& context,
                           const TexturePool& texturePool,
                           const TransformState& state,
                           const std::shared_ptr<UpdateParameters>& updateParameters,
                           const RenderTree& /*renderTree*/,
                           UniqueChangeRequestVec& changes) {
    // Find the DEM source if we haven't already
    if (!demSource && !impl->sourceID.empty()) {
        demSource = orchestrator.getRenderSource(impl->sourceID);
        if (!demSource) {
            Log::Warning(Event::Render, "Terrain could not find DEM source: " + impl->sourceID);
        }
    }

    // Create layer group if we don't have one (including after rebuild)
    if (!layerGroup) {
        if (auto layerGroup_ = context.createLayerGroup(TERRAIN_LAYER_INDEX, /*initialCapacity=*/1, "terrain", false)) {
            layerGroup = std::move(layerGroup_);
            activateLayerGroup(true, changes);
        } else {
            Log::Error(Event::Render, "Failed to create terrain layer group");
            return;
        }
    }

    // Depth-pass twin of the terrain layer group; not activated in the
    // orchestrator, rendered only by renderDepth into the depth target
    if (!depthLayerGroup) {
        depthLayerGroup = context.createLayerGroup(TERRAIN_LAYER_INDEX, /*initialCapacity=*/1, "terrain-depth", false);
    }

    // Create tweaker if we don't have one
    if (!tweaker) {
        tweaker = std::make_unique<TerrainLayerTweaker>(this);
    }

    // If we don't have a DEM source, we can't create terrain drawables
    if (!demSource) {
        return;
    }

    // Get tiles from the DEM source
    auto renderTiles = demSource->getRawRenderTiles();
    if (renderTiles->empty()) {
        return;
    }

    // Cast to LayerGroup for addDrawable
    auto* lg = static_cast<LayerGroup*>(layerGroup.get());
    if (!lg) {
        return;
    }

    // Decode and cache the DEM textures of loaded DEM tiles
    ++demUpdateCounter;
    for (const auto& renderTile : *renderTiles) {
        const auto& tile = renderTile.getTile();
        if (tile.kind != Tile::Kind::RasterDEM) {
            continue;
        }
        auto* demTile = const_cast<RasterDEMTile*>(static_cast<const RasterDEMTile*>(&tile));
        auto* hillshadeBucket = demTile->getBucket();
        const auto* demData = hillshadeBucket ? &hillshadeBucket->getDEMData() : nullptr;
        if (demData && demData->getImagePtr() && !demData->getImagePtr()->size.isEmpty()) {
            // All tiles come from the same raster-dem source, so they share one
            // encoding and DEM dimension
            demUnpackVector = demData->getUnpackVector();
            demDim = demData->dim;
            if (auto existing = demTextures.find(renderTile.id); existing != demTextures.end()) {
                existing->second.lastUsed = demUpdateCounter;
            } else if (auto texture = createDEMTexture(context, *demData)) {
                // Keep the texture available for elevation sampling by non-draped layers
                demTextures[renderTile.id] = {texture, demData->dim, demUpdateCounter};
            }
        }
    }

    // The mesh (and drape) tile set: an elevation-aware, LOD-based ideal cover
    // taken straight from the view, not from the DEM source's loaded tiles - so a
    // sparse DEM's low-zoom fallback tiles don't drag the drape resolution down
    // (blurry roads) or leave uncovered areas as skirt. DEM/drape textures fall
    // back to ancestors per tile below while exact tiles load. tileCover already
    // limits to the frustum (elevation included), so no extra cull is needed.
    std::set<UnwrappedTileID> meshTiles = computeMeshCover(state, updateParameters);

    // Drop drawables and cached DEM textures for tiles that left the mesh tile
    // set, keeping everything else intact between frames
    std::unordered_set<OverscaledTileID> currentTiles;
    for (const auto& id : meshTiles) {
        currentTiles.emplace(id.canonical.z, id.wrap, id.canonical);
    }
    lg->removeDrawablesIf(
        [&](gfx::Drawable& drawable) { return drawable.getTileID() && !currentTiles.contains(*drawable.getTileID()); });
    auto* depthLg = static_cast<LayerGroup*>(depthLayerGroup.get());
    if (depthLg) {
        depthLg->removeDrawablesIf([&](gfx::Drawable& drawable) {
            return drawable.getTileID() && !currentTiles.contains(*drawable.getTileID());
        });
    }
    for (auto it = tilesWithDrawables.begin(); it != tilesWithDrawables.end();) {
        if (!currentTiles.contains(it->first)) {
            drawableDemCoords.erase(it->first);
            it = tilesWithDrawables.erase(it);
        } else {
            ++it;
        }
    }
    // Retain cached DEM textures that are related to the current tile set so they
    // can serve as ancestor fallbacks while exact tiles load (as maplibre-gl-js
    // retains terrain tiles in its source cache); drop unrelated ones
    for (auto it = demTextures.begin(); it != demTextures.end();) {
        bool related = false;
        for (const auto& current : currentTiles) {
            const UnwrappedTileID unwrapped = current.toUnwrapped();
            if (unwrapped == it->first || unwrapped.isChildOf(it->first) || it->first.isChildOf(unwrapped)) {
                related = true;
                break;
            }
        }
        it = related ? std::next(it) : demTextures.erase(it);
    }
    // Cap the cache: ancestor/descendant relations accumulate while browsing
    // (zooming makes whole chains "related"), which previously grew past 2GB
    // of DEM textures and overflowed/OOMed. Evict least-recently-used entries
    // that were not used this frame until the cache is back under budget.
    if (demTextures.size() > maxDEMTextures) {
        std::vector<std::pair<uint64_t, UnwrappedTileID>> evictable;
        for (const auto& [id, entry] : demTextures) {
            if (entry.lastUsed != demUpdateCounter) {
                evictable.emplace_back(entry.lastUsed, id);
            }
        }
        std::sort(evictable.begin(), evictable.end());
        for (const auto& [lastUsed, id] : evictable) {
            if (demTextures.size() <= maxDEMTextures) {
                break;
            }
            demTextures.erase(id);
        }
    }

    // Create terrain drawables for each mesh tile
    for (const auto& unwrapped : meshTiles) {
        const OverscaledTileID tileID(unwrapped.canonical.z, unwrapped.wrap, unwrapped.canonical);

        // Skip if the tile already has a drawable bound to its own DEM
        if (const auto existing = tilesWithDrawables.find(tileID);
            existing != tilesWithDrawables.end() && existing->second == 2) {
            continue;
        }

        // Resolve the DEM texture: the tile's own decoded DEM if available,
        // otherwise the closest cached ancestor as a fallback so the terrain
        // mesh stays up while the tile loads (as maplibre-gl-js does),
        // otherwise the flat placeholder
        std::shared_ptr<gfx::Texture2D> demTexture;
        // {scale, x offset, y offset, DEM dim}: maps tile-local coords (0..EXTENT)
        // into the bound DEM tile's normalized space, matching getTerrainData so
        // the terrain mesh and the elevated layers sample identically. The DEM
        // dimension rides in .w for the shader's get_elevation() call.
        std::array<float, 4> demCoords{{1.0f / util::EXTENT, 0.0f, 0.0f, static_cast<float>(demDim)}};
        uint8_t demTier = 0;

        if (auto cached = demTextures.find(unwrapped); cached != demTextures.end()) {
            cached->second.lastUsed = demUpdateCounter;
            demTexture = cached->second.texture;
            demTier = 2;
        } else {
            // Fall back to the closest cached ancestor DEM
            const UnwrappedTileID* ancestorID = nullptr;
            DEMTextureEntry* ancestorEntry = nullptr;
            int bestZoom = -1;
            for (auto& [candidate, entry] : demTextures) {
                if (candidate != unwrapped && unwrapped.isChildOf(candidate) &&
                    static_cast<int>(candidate.canonical.z) > bestZoom) {
                    bestZoom = candidate.canonical.z;
                    ancestorID = &candidate;
                    ancestorEntry = &entry;
                    demTexture = entry.texture;
                }
            }
            if (!demTexture) {
                // No DEM at all yet: render the mesh flat with the placeholder
                // DEM so the draped map still shows (a briefly flat area is
                // less jarring than a hole in the terrain)
                demTexture = getPlaceholderDEMTexture(context);
                if (!demTexture) {
                    continue;
                }
            } else {
                ancestorEntry->lastUsed = demUpdateCounter;
                demTier = 1;
                const auto off = demSubTileOffset(unwrapped.canonical, ancestorID->canonical);
                demCoords = {{1.0f / (util::EXTENT * off.scale),
                              off.dx / off.scale,
                              off.dy / off.scale,
                              static_cast<float>(demDim)}};
            }
        }

        // If a drawable already exists for this tile, keep it until a higher
        // DEM quality tier becomes available, then replace it
        if (const auto existing = tilesWithDrawables.find(tileID); existing != tilesWithDrawables.end()) {
            if (existing->second >= demTier) {
                continue;
            }
            lg->removeDrawablesIf(
                [&](gfx::Drawable& drawable) { return drawable.getTileID() && *drawable.getTileID() == tileID; });
            if (depthLg) {
                depthLg->removeDrawablesIf(
                    [&](gfx::Drawable& drawable) { return drawable.getTileID() && *drawable.getTileID() == tileID; });
            }
            tilesWithDrawables.erase(existing);
        }
        drawableDemCoords[tileID] = demCoords;

        // Create terrain drawable for this tile
        const auto renderTarget = texturePool.getRenderTarget(unwrapped);
        if (!renderTarget) {
            continue;
        }
        auto drawable = createDrawableForTile(context, shaders, tileID, demTexture, renderTarget->getTexture());
        if (drawable) {
            lg->addDrawable(std::move(drawable));
            tilesWithDrawables[tileID] = demTier;
            if (depthLg) {
                if (auto depthDrawable = createDrawableForTile(
                        context, shaders, tileID, demTexture, nullptr, /*depthPass=*/true)) {
                    depthLg->addDrawable(std::move(depthDrawable));
                }
            }
        }
    }

    // Debug-only, off by default: gate the whole above-ground check (per-frame free-camera +
    // elevation sampling) on the flag so it costs nothing unless explicitly enabled.
    if (updateParameters && updateParameters->debugAboveGroundLog) {
        logAboveGroundMargin(state);
    }
}

void RenderTerrain::logAboveGroundMargin(const TransformState& state) {
    // Log the camera eye's clearance over the *rendered* terrain surface each frame (throttled),
    // so the flight/interaction tests can report when the sea-level-anchored camera dips below
    // terrain - the FPV underground/white artifact (TERRAIN.md Phase 4). Groundwork for a future
    // terrain-anchored camera: a concrete, measurable "how far under, and where" signal.
    if (!demSource) {
        return;
    }
    const auto fco = state.getFreeCameraOptions();
    const auto loc = fco.getLocation(); // eye Lat/Lng + altitude in metres (engine's own conversion)
    if (!fco.position || !loc) {
        return;
    }
    const double now = util::MonotonicTimer::now().count();
    if (now - lastAboveGroundLog < kAboveGroundLogInterval) {
        return;
    }
    lastAboveGroundLog = now;

    // Sample the rendered (exaggerated) terrain height directly under the eye. The eye's
    // horizontal position is the free-camera mercator x/y (0..1); getElevation walks to the best
    // loaded DEM ancestor, so a deep sample zoom just picks the finest tile available there.
    const auto& p = *fco.position;
    constexpr int sampleZoom = 14;
    const double n = std::pow(2.0, sampleZoom);
    const double fx = p[0] * n;
    const double fy = p[1] * n;
    const auto tx = static_cast<int64_t>(std::floor(fx));
    const auto ty = static_cast<int64_t>(std::floor(fy));
    const UnwrappedTileID sampleTile(static_cast<uint8_t>(sampleZoom), tx, ty);
    const auto localX = static_cast<float>((fx - static_cast<double>(tx)) * util::EXTENT);
    const auto localY = static_cast<float>((fy - static_cast<double>(ty)) * util::EXTENT);

    const double groundM = getElevationWithExaggeration(sampleTile, localX, localY);
    const double marginM = loc->altitude - groundM;
    // Only log when the camera is near or below the terrain - the interesting case, and low
    // noise (normal viewing sits km above). A DEM miss reads as groundM==0 -> large positive
    // margin, so it also stays below this gate and is not mistaken for real clearance.
    if (marginM >= kAboveGroundAlertM) {
        return;
    }
    std::ostringstream os;
    os << std::fixed << std::setprecision(1) << "ABOVE-GROUND marginM=" << marginM << " camAltM=" << loc->altitude
       << " groundM=" << groundM << " under=" << (marginM < 0.0 ? 1 : 0) << std::setprecision(4)
       << " zoom=" << state.getZoom() << " pitch=" << state.getPitch() << " lng=" << loc->location.longitude()
       << " lat=" << loc->location.latitude();
    Log::Info(Event::Render, os.str());
}

float RenderTerrain::getElevation(const UnwrappedTileID& tileID, float x, float y) const {
    if (!demSource) {
        return 0.0f;
    }

    // Find the DEM tile matching the requested tile, or its closest available ancestor
    const auto renderTiles = demSource->getRawRenderTiles();
    const RenderTile* demRenderTile = nullptr;
    int bestZoom = -1;
    for (const auto& renderTile : *renderTiles) {
        const UnwrappedTileID& candidate = renderTile.id;
        if ((candidate == tileID || tileID.isChildOf(candidate)) &&
            static_cast<int>(candidate.canonical.z) > bestZoom) {
            bestZoom = candidate.canonical.z;
            demRenderTile = &renderTile;
        }
    }
    if (!demRenderTile) {
        return 0.0f;
    }

    const auto& tile = demRenderTile->getTile();
    if (tile.kind != Tile::Kind::RasterDEM) {
        return 0.0f;
    }
    auto* demTile = const_cast<RasterDEMTile*>(static_cast<const RasterDEMTile*>(&tile));
    auto* bucket = demTile->getBucket();
    if (!bucket) {
        return 0.0f;
    }
    const auto& demData = bucket->getDEMData();
    if (!demData.getImagePtr() || demData.dim <= 0) {
        return 0.0f;
    }

    // Map the tile-local coordinate into the (possibly ancestor) DEM tile
    const UnwrappedTileID& demTileID = demRenderTile->id;
    const auto off = demSubTileOffset(tileID.canonical, demTileID.canonical);
    const float xInDem = (off.dx * util::EXTENT + x) / off.scale;
    const float yInDem = (off.dy * util::EXTENT + y) / off.scale;

    // Bilinear interpolation of the DEM texels, as in maplibre-gl-js Terrain.getDEMElevation
    const float dim = static_cast<float>(demData.dim);
    const float px = util::clamp(xInDem / util::EXTENT * dim, 0.0f, dim - 1.0f);
    const float py = util::clamp(yInDem / util::EXTENT * dim, 0.0f, dim - 1.0f);
    const auto x0 = static_cast<int32_t>(std::floor(px));
    const auto y0 = static_cast<int32_t>(std::floor(py));
    const float fx = px - static_cast<float>(x0);
    const float fy = py - static_cast<float>(y0);
    const float tl = static_cast<float>(demData.get(x0, y0));
    const float tr = static_cast<float>(demData.get(x0 + 1, y0));
    const float bl = static_cast<float>(demData.get(x0, y0 + 1));
    const float br = static_cast<float>(demData.get(x0 + 1, y0 + 1));
    const float top = tl + (tr - tl) * fx;
    const float bottom = bl + (br - bl) * fx;
    return top + (bottom - top) * fy;
}

float RenderTerrain::getElevationWithExaggeration(const UnwrappedTileID& tileID, float x, float y) const {
    return getElevation(tileID, x, y) * getExaggeration();
}

std::optional<RenderTerrain::TerrainData> RenderTerrain::getTerrainData(const UnwrappedTileID& tileID) const {
    // Find the DEM texture matching the requested tile, or its closest available ancestor
    const UnwrappedTileID* demTileID = nullptr;
    const DEMTextureEntry* entry = nullptr;
    int bestZoom = -1;
    for (const auto& [candidate, candidateEntry] : demTextures) {
        if ((candidate == tileID || tileID.isChildOf(candidate)) &&
            static_cast<int>(candidate.canonical.z) > bestZoom) {
            bestZoom = candidate.canonical.z;
            demTileID = &candidate;
            entry = &candidateEntry;
        }
    }
    if (!entry || !entry->texture) {
        return std::nullopt;
    }

    // Map tile-local coordinates (0..EXTENT) of the requested tile into
    // normalized coordinates (0..1) of the (possibly ancestor) DEM tile
    const auto off = demSubTileOffset(tileID.canonical, demTileID->canonical);

    return TerrainData{
        .demTexture = entry->texture,
        .demCoords = {{1.0f / (util::EXTENT * off.scale), off.dx / off.scale, off.dy / off.scale, 0.0f}},
        .demDim = static_cast<float>(entry->dim),
    };
}

const std::shared_ptr<gfx::Texture2D>& RenderTerrain::getPlaceholderDEMTexture(gfx::Context& context) {
    if (!placeholderDEMTexture) {
        auto image = std::make_shared<PremultipliedImage>(Size{1, 1});
        std::memset(image->data.get(), 0, image->bytes());
        placeholderDEMTexture = context.createTexture2D();
        placeholderDEMTexture->setImage(image);
        placeholderDEMTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Nearest,
                                                        .wrapU = gfx::TextureWrapType::Clamp,
                                                        .wrapV = gfx::TextureWrapType::Clamp});
    }
    return placeholderDEMTexture;
}

void RenderTerrain::renderDepth(RenderOrchestrator& orchestrator,
                                const RenderTree& renderTree,
                                PaintParameters& parameters) {
    if (!depthLayerGroup || depthLayerGroup->empty()) {
        return;
    }
    prepareDepthTarget(parameters);
    if (!depthRenderTarget) {
        return;
    }
    depthRenderTarget->render(orchestrator, renderTree, parameters);
}

void RenderTerrain::prepareDepthTarget(PaintParameters& parameters) {
    // Called at the start of the render (before the upload phase) as well as from
    // renderDepth, so the depth target already exists when the symbol tweaker binds
    // getDepthTexture() for this frame. Creating it lazily in renderDepth alone left
    // the symbols bound to the far-plane placeholder for that frame - permanently so
    // in single-frame renders like the render tests, where terrain occlusion then
    // never engaged.
    const Size size = parameters.backend.getDefaultRenderable().getSize();
    if (size.isEmpty()) {
        // Early frames can run before the surface has a real size; a degenerate
        // render target here would hand the symbol tweaker a broken texture
        return;
    }
    if (!depthRenderTarget || !depthRenderTarget->getTexture() || depthRenderTarget->getTexture()->getSize() != size) {
        depthRenderTarget = parameters.context.createRenderTarget(
            size, gfx::TextureChannelDataType::UnsignedByte, /*stencil=*/false);
        if (!depthRenderTarget) {
            return;
        }
        // Far plane everywhere the terrain does not cover (unpack_depth(1,1,1,1) ~ 1.0)
        depthRenderTarget->setClearColor(Color::white());
    }
    // (Re)attach the depth layer group; it may not have existed yet when the
    // target was first created on an early frame
    if (depthLayerGroup) {
        depthRenderTarget->addLayerGroup(depthLayerGroup, /*replace=*/true);
    }
}

const std::shared_ptr<gfx::Texture2D>& RenderTerrain::getDepthTexture(gfx::Context& context) {
    if (depthRenderTarget && depthRenderTarget->getTexture()) {
        return depthRenderTarget->getTexture();
    }
    if (!placeholderDepthTexture) {
        // Far-plane packed depth: symbols compare as visible until the pass runs
        auto image = std::make_shared<PremultipliedImage>(Size{1, 1});
        std::memset(image->data.get(), 0xFF, image->bytes());
        placeholderDepthTexture = context.createTexture2D();
        placeholderDepthTexture->setImage(image);
        placeholderDepthTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Nearest,
                                                          .wrapU = gfx::TextureWrapType::Clamp,
                                                          .wrapV = gfx::TextureWrapType::Clamp});
    }
    return placeholderDepthTexture;
}

float RenderTerrain::getExaggeration() const {
    return impl->exaggeration;
}

const std::string& RenderTerrain::getSourceID() const {
    return impl->sourceID;
}

bool RenderTerrain::isEnabled() const {
    return !impl->sourceID.empty();
}

const RenderTerrain::TerrainMesh& RenderTerrain::getMesh(gfx::Context& context) {
    if (!mesh) {
        generateMesh(context);
    }
    return *mesh;
}

void RenderTerrain::generateMesh(gfx::Context& /*context*/) {
    // A regular grid mesh (reused for every tile, displaced by the DEM in the
    // vertex shader) plus a skirt: each tile edge is duplicated into a curtain
    // that the shader drops by u_ele_delta, hiding the cracks between neighbouring
    // tiles at different zoom levels. Ported from maplibre-gl-js Terrain
    // getTerrainMesh()/_buildSkirts().
    const size_t gridSize = MESH_SIZE;
    const size_t vps = gridSize + 1; // vertices per side
    const float step = static_cast<float>(util::EXTENT) / static_cast<float>(gridSize);

    std::vector<int16_t> vertices;
    std::vector<uint16_t> indices;

    // Each vertex is 4 shorts: x, y, skirt flag (0 = surface, 1 = skirt),
    // unused. uv is derived from x,y in the shader, so the 3rd/4th shorts are free
    // to carry the skirt flag (the native analog of gl-js Pos3d.z).
    const auto addVert = [&](float x, float y, int16_t skirt) {
        vertices.push_back(static_cast<int16_t>(x));
        vertices.push_back(static_cast<int16_t>(y));
        vertices.push_back(skirt);
        vertices.push_back(0);
    };

    // Surface grid
    for (size_t y = 0; y < vps; ++y) {
        for (size_t x = 0; x < vps; ++x) {
            addVert(x * step, y * step, 0);
        }
    }
    for (size_t y = 0; y < gridSize; ++y) {
        for (size_t x = 0; x < gridSize; ++x) {
            const uint16_t topLeft = static_cast<uint16_t>(y * vps + x);
            const uint16_t topRight = static_cast<uint16_t>(topLeft + 1);
            const uint16_t bottomLeft = static_cast<uint16_t>((y + 1) * vps + x);
            const uint16_t bottomRight = static_cast<uint16_t>(bottomLeft + 1);
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    // Top/bottom skirt rows (reference the grid's top/bottom edge rows)
    const auto extent = static_cast<float>(util::EXTENT);
    const uint16_t offsetTop = static_cast<uint16_t>(vertices.size() / 4);
    const uint16_t offsetTopEdge = 0;
    const uint16_t offsetBottom = static_cast<uint16_t>(offsetTop + vps);
    const uint16_t offsetBottomEdge = static_cast<uint16_t>(vps * gridSize);
    for (size_t x = 0; x < vps; ++x) {
        addVert(x * step, 0.0f, 1);
    }
    for (size_t x = 0; x < vps; ++x) {
        addVert(x * step, extent, 1);
    }
    for (uint16_t x = 0; x < gridSize; ++x) {
        indices.insert(indices.end(),
                       {static_cast<uint16_t>(offsetBottomEdge + x),
                        static_cast<uint16_t>(offsetBottom + x),
                        static_cast<uint16_t>(offsetBottom + x + 1),
                        static_cast<uint16_t>(offsetBottomEdge + x),
                        static_cast<uint16_t>(offsetBottom + x + 1),
                        static_cast<uint16_t>(offsetBottomEdge + x + 1),
                        static_cast<uint16_t>(offsetTopEdge + x),
                        static_cast<uint16_t>(offsetTop + x + 1),
                        static_cast<uint16_t>(offsetTop + x),
                        static_cast<uint16_t>(offsetTopEdge + x),
                        static_cast<uint16_t>(offsetTopEdge + x + 1),
                        static_cast<uint16_t>(offsetTop + x + 1)});
    }

    // Left/right skirt frames (self-contained strips of paired surface/skirt verts)
    const uint16_t offsetLeft = static_cast<uint16_t>(vertices.size() / 4);
    const uint16_t offsetRight = static_cast<uint16_t>(offsetLeft + vps * 2);
    for (int edge = 0; edge <= 1; ++edge) {
        for (size_t y = 0; y < vps; ++y) {
            for (int16_t z = 0; z <= 1; ++z) {
                addVert(static_cast<float>(edge) * extent, y * step, z);
            }
        }
    }
    for (uint16_t y = 0; y < gridSize * 2; y += 2) {
        indices.insert(indices.end(),
                       {static_cast<uint16_t>(offsetLeft + y),
                        static_cast<uint16_t>(offsetLeft + y + 1),
                        static_cast<uint16_t>(offsetLeft + y + 3),
                        static_cast<uint16_t>(offsetLeft + y),
                        static_cast<uint16_t>(offsetLeft + y + 3),
                        static_cast<uint16_t>(offsetLeft + y + 2),
                        static_cast<uint16_t>(offsetRight + y),
                        static_cast<uint16_t>(offsetRight + y + 3),
                        static_cast<uint16_t>(offsetRight + y + 1),
                        static_cast<uint16_t>(offsetRight + y),
                        static_cast<uint16_t>(offsetRight + y + 2),
                        static_cast<uint16_t>(offsetRight + y + 3)});
    }

    mesh = TerrainMesh{nullptr, // vertexBuffer - created when building the drawable
                       nullptr, // indexBuffer - created when building the drawable
                       vertices.size() / 4,
                       indices.size(),
                       std::move(vertices),
                       std::move(indices)};
}

std::shared_ptr<gfx::Texture2D> RenderTerrain::createDEMTexture(gfx::Context& context, const DEMData& demData) {
    // Get the DEM image data
    const auto& imagePtr = demData.getImagePtr();
    if (!imagePtr || imagePtr->size.isEmpty()) {
        Log::Warning(Event::Render, "DEM data has no image");
        return nullptr;
    }

    // Create a new texture
    auto texture = context.createTexture2D();
    if (!texture) {
        Log::Error(Event::Render, "Failed to create DEM texture");
        return nullptr;
    }

    // Set the image data
    texture->setImage(imagePtr);

    // Nearest filtering: the packed Terrain-RGB/Terrarium DEM cannot be hardware
    // interpolated (blending the encoded bytes does not blend the decoded
    // elevations), so shaders decode each texel and interpolate in meters via
    // get_elevation(). This matches maplibre-gl-js, which binds the DEM NEAREST.
    texture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Nearest,
                                      .wrapU = gfx::TextureWrapType::Clamp,
                                      .wrapV = gfx::TextureWrapType::Clamp});

    return texture;
}

std::unique_ptr<gfx::Drawable> RenderTerrain::createDrawableForTile(gfx::Context& context,
                                                                    gfx::ShaderRegistry& shaders,
                                                                    const OverscaledTileID& tileID,
                                                                    std::shared_ptr<gfx::Texture2D> demTexture,
                                                                    std::shared_ptr<gfx::Texture2D> mapTexture,
                                                                    bool depthPass) {
    // Ensure mesh is generated
    const auto& terrainMesh = getMesh(context);

    if (terrainMesh.vertices.empty() || terrainMesh.indices.empty()) {
        Log::Error(Event::Render, "Terrain mesh is empty, cannot create drawable");
        return nullptr;
    }

    // Get terrain shader
    auto terrainShader = context.getGenericShader(shaders, depthPass ? "TerrainDepthShader" : "TerrainShader");
    if (!terrainShader) {
        // The depth shader is not registered on all backends yet; symbols
        // then sample the far-plane placeholder and stay visible
        if (!depthPass) {
            Log::Error(Event::Render, "Terrain shader not found");
        }
        return nullptr;
    }

    // Create drawable builder
    auto builder = context.createDrawableBuilder(depthPass ? "terrain-depth-tile" : "terrain-tile");
    if (!builder) {
        Log::Error(Event::Render, "Failed to create drawable builder for terrain tile");
        return nullptr;
    }

    // The drape pass uses the Translucent render pass because it renders in
    // forward order (high index = front), unlike Opaque which renders reversed.
    builder->setShader(terrainShader);
    builder->setRenderPass(RenderPass::Translucent);
    if (depthPass) {
        // The depth pass renders packed depth with real depth testing so the
        // nearest surface wins, into the terrain depth target (renderDepth)
        builder->setDepthType(gfx::DepthMaskType::ReadWrite);
        builder->setColorMode(gfx::ColorMode::unblended());
        builder->setEnableDepth(true);
        builder->setIs3D(true);
    } else {
        // The terrain surface is 3D geometry, so it tests and writes depth: nearer
        // terrain occludes farther terrain, and - crucially - occludes the skirt
        // curtains hanging below each tile edge, so the skirts only show through
        // the cracks they exist to fill rather than drawing over the surface.
        //
        // Symbols (and other layers that occlude against terrain via the depth
        // texture) must not main-depth-test against this surface, or they would be
        // culled by the terrain they sit on - the symbol tweaker disables their
        // depth test while terrain is enabled.
        builder->setDepthType(gfx::DepthMaskType::ReadWrite);
        builder->setColorMode(gfx::ColorMode::unblended());
        builder->setEnableDepth(true);
        builder->setIs3D(true);
    }

    // Set vertex data - copy vertices to raw buffer
    std::vector<uint8_t> vertexData(terrainMesh.vertices.size() * sizeof(int16_t));
    std::memcpy(vertexData.data(), terrainMesh.vertices.data(), vertexData.size());
    builder->setRawVertices(std::move(vertexData), terrainMesh.vertexCount, gfx::AttributeDataType::Short4);

    // Set index data and segments
    // Create a single segment covering the entire terrain mesh
    SegmentVector segments;
    segments.emplace_back(0,                       // vertex offset
                          0,                       // index offset
                          terrainMesh.vertexCount, // vertex count
                          terrainMesh.indexCount); // index count

    std::vector<uint16_t> indexData = terrainMesh.indices;
    builder->setSegments(gfx::Triangles(), std::move(indexData), segments.data(), segments.size());

    // Set the DEM texture
    if (demTexture) {
        builder->setTexture(demTexture, 0); // Texture index 0 for DEM
    }

    // The depth pass samples only the DEM and writes packed depth, so it has no
    // map texture by design; only the draped pass binds the drape render target
    if (!depthPass) {
        if (mapTexture) {
            builder->setTexture(mapTexture, 1); // Texture index 1 for map
        } else {
            Log::Warning(Event::Render, "No drape texture for terrain tile " + util::toString(tileID));
        }
    }

    // Flush to create the drawable
    builder->flush(context);

    // Get the drawable
    auto drawables = builder->clearDrawables();
    if (drawables.empty()) {
        Log::Error(Event::Render, "Failed to create terrain drawable for tile");
        return nullptr;
    }

    // Set tile ID on the drawable
    auto& drawable = drawables[0];
    drawable->setTileID(tileID);

    return std::move(drawable);
}

void RenderTerrain::activateLayerGroup(bool activate, UniqueChangeRequestVec& changes) {
    if (layerGroup) {
        if (activate) {
            changes.emplace_back(std::make_unique<AddLayerGroupRequest>(layerGroup));
        } else {
            changes.emplace_back(std::make_unique<RemoveLayerGroupRequest>(layerGroup));
        }
    }
}

void RenderTerrain::deactivate(UniqueChangeRequestVec& changes) {
    // depthLayerGroup / depthRenderTarget are owned by this RenderTerrain and
    // released with it; only the mesh layerGroup is registered separately with
    // the orchestrator, so that is all we need to unregister here.
    activateLayerGroup(false, changes);
}

} // namespace mbgl
