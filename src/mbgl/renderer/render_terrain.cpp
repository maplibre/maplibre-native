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
#if MLN_RENDER_BACKEND_OPENGL
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/texture_2d_array.hpp>
#include <mbgl/gl/drawable_gl.hpp> // gl::DrawableGL::setArrayTexture for the instanced depth DEM
#endif
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
#include <mbgl/util/geo.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/monotonic_timer.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/map/camera.hpp>
#include <mbgl/util/convert.hpp> // util::cast for the instanced depth UBO matrix
#include <mbgl/util/hash.hpp>    // util::hash_combine for the depth-instance set signature
#include <mbgl/gfx/vertex_attribute.hpp> // VertexAttributeArray for the a_instance attribute

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
#if MLN_RENDER_BACKEND_OPENGL
                // Also pack this tile's DEM into the array for the (upcoming) instanced
                // depth pass. Additive: the per-tile texture above is still the fallback.
                packDEMArrayLayer(context, renderTile.id, *demData);
#endif
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

    // Cap the mesh tile count: keep those nearest the map center, drop the farthest
    // (the horizon tiles a high tilt pulls in). Everything downstream - drape
    // targets, re-renders, depth draws - scales with this count.
    // Per-mode cap (TerrainLoadBudget::maxMeshTiles): Quality keeps a generous cap so terrain
    // render distance stays long; Balanced/Performance trade distance for frame time.
    const size_t maxMeshTiles = updateParameters ? terrainLoadBudget(updateParameters->terrainLoadMode).maxMeshTiles
                                                 : 0;
    if (maxMeshTiles > 0 && meshTiles.size() > maxMeshTiles) {
        // Map center in normalized web-mercator [0,1] (standard projection)
        const LatLng center = state.getLatLng();
        const double cx = center.longitude() / 360.0 + 0.5;
        const double latRad = util::deg2rad(center.latitude());
        const double cy = 0.5 - std::log(std::tan(M_PI / 4.0 + latRad / 2.0)) / (2.0 * M_PI);

        const auto tileDist2 = [&](const UnwrappedTileID& id) {
            const double scale = static_cast<double>(1u << id.canonical.z);
            const double tx = (static_cast<double>(id.canonical.x) + 0.5) / scale + id.wrap;
            const double ty = (static_cast<double>(id.canonical.y) + 0.5) / scale;
            const double dx = tx - cx;
            const double dy = ty - cy;
            return dx * dx + dy * dy;
        };

        std::vector<UnwrappedTileID> sorted(meshTiles.begin(), meshTiles.end());
        std::partial_sort(sorted.begin(),
                          sorted.begin() + static_cast<std::ptrdiff_t>(maxMeshTiles),
                          sorted.end(),
                          [&](const UnwrappedTileID& a, const UnwrappedTileID& b) {
                              return tileDist2(a) < tileDist2(b);
                          });
        meshTiles = std::set<UnwrappedTileID>(sorted.begin(),
                                              sorted.begin() + static_cast<std::ptrdiff_t>(maxMeshTiles));
    }

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
        if (depthLg->removeDrawablesIf([&](gfx::Drawable& drawable) {
                return drawable.getTileID() && !currentTiles.contains(*drawable.getTileID());
            }) > 0) {
            depthDirty = true;
        }
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
        if (related) {
            it = std::next(it);
        } else {
#if MLN_RENDER_BACKEND_OPENGL
            freeDEMArrayLayer(it->first);
#endif
            it = demTextures.erase(it);
        }
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
#if MLN_RENDER_BACKEND_OPENGL
            freeDEMArrayLayer(id);
#endif
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
        // DEM tile whose texture / array-layer this tile uses. Only *read* by the GL
        // instanced-depth block below, so mark it maybe_unused: other backends keep the
        // per-tile depth drawables and would otherwise fail -Wunused-but-set-variable.
        [[maybe_unused]] const UnwrappedTileID* demTileUsed = nullptr;

        if (auto cached = demTextures.find(unwrapped); cached != demTextures.end()) {
            cached->second.lastUsed = demUpdateCounter;
            demTexture = cached->second.texture;
            demTier = 2;
            demTileUsed = &unwrapped;
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
                demTileUsed = ancestorID;
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
                depthDirty = true;
            }
            tilesWithDrawables.erase(existing);
        }
        drawableDemCoords[tileID] = demCoords;
#if MLN_RENDER_BACKEND_OPENGL
        {
            float layer = -1.0f;
            if (demTileUsed) {
                if (const auto la = demArrayLayer.find(*demTileUsed); la != demArrayLayer.end()) {
                    layer = static_cast<float>(la->second);
                }
            }
            drawableDemLayer[tileID] = layer;
        }
#endif

        // Create terrain drawable for this tile
        const auto renderTarget = texturePool.getRenderTarget(unwrapped);
        if (!renderTarget) {
            continue;
        }
        auto drawable = createDrawableForTile(context, shaders, tileID, demTexture, renderTarget->getTexture());
        if (drawable) {
            lg->addDrawable(std::move(drawable));
            tilesWithDrawables[tileID] = demTier;
#if !MLN_RENDER_BACKEND_OPENGL
            // Non-GL backends: one depth drawable per tile (no instancing path there).
            if (depthLg) {
                if (auto depthDrawable = createDrawableForTile(
                        context, shaders, tileID, demTexture, nullptr, /*depthPass=*/true)) {
                    depthLg->addDrawable(std::move(depthDrawable));
                    depthDirty = true;
                }
            }
#endif
        }
    }

    // Debug-only, off by default: gate the whole above-ground check (per-frame free-camera +
    // elevation sampling) on the flag so it costs nothing unless explicitly enabled.
    if (updateParameters && updateParameters->debugAboveGroundLog) {
        logAboveGroundMargin(state);
    }

#if MLN_RENDER_BACKEND_OPENGL
    // GL: collect the per-instance (tile, dem_coords, dem_layer) list for the whole mesh tile
    // set and rebuild the single instanced depth drawable only when that set changes (its
    // transforms refresh every frame in updateInstancedDepthUBO). Tiles without a packed DEM
    // array layer (-1) are skipped - they briefly miss depth occlusion, the same tolerance the
    // old ancestor/placeholder fallback had.
    {
        std::vector<DepthInstance> instances;
        instances.reserve(meshTiles.size());
        std::size_t sig = 0;
        for (const auto& unwrapped : meshTiles) {
            const OverscaledTileID tileID(unwrapped.canonical.z, unwrapped.wrap, unwrapped.canonical);
            const auto lc = drawableDemLayer.find(tileID);
            if (lc == drawableDemLayer.end() || lc->second < 0.0f || instances.size() >= maxDepthInstances) {
                continue;
            }
            const auto cc = drawableDemCoords.find(tileID);
            const std::array<float, 4> coords =
                cc != drawableDemCoords.end()
                    ? cc->second
                    : std::array<float, 4>{{1.0f / util::EXTENT, 0.0f, 0.0f, static_cast<float>(demDim)}};
            instances.push_back({tileID, coords, lc->second});
            util::hash_combine(sig, std::hash<OverscaledTileID>{}(tileID));
            util::hash_combine(sig, static_cast<int>(lc->second));
        }
        depthInstances = std::move(instances);
        if (sig != depthInstanceSignature) {
            depthInstanceSignature = sig;
            rebuildInstancedDepthDrawable(context, shaders);
            depthDirty = true;
        }
    }
#endif
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
    // The packed-depth output is a function of the camera projection and the terrain mesh set
    // only. When neither changed since the last depth render, the existing depth texture is
    // still correct - skip the whole pass. This is what makes a static scene cheap; the depth
    // is redrawn only on camera movement or a mesh/tile change. (From 604f293; without this
    // gate the pass ran every frame, which is the state that flickered.)
    const mat4& proj = parameters.transformParams.projMatrix;
    const bool cameraMoved = !lastDepthProjMatrix || *lastDepthProjMatrix != proj;
    if (!depthDirty && !cameraMoved) {
        return;
    }

#if MLN_RENDER_BACKEND_OPENGL
    // Instanced depth pass: refresh the per-instance UBO array (camera-dependent transforms)
    // and bind the packed DEM array to the unit the shader's u_dem_array sampler expects. The
    // instanced drawable carries no gfx textures, so nothing else touches this unit during the
    // depth render. (Bind integration is the main on-device shakeout item - Texture2DArray is
    // GL-only and outside the gfx texture abstraction.)
    updateInstancedDepthUBO(parameters);
    // The DEM array is bound by the instanced drawable itself (DrawableGL::setArrayTexture ->
    // bindTextures), so no manual bind here.
#endif
    depthRenderTarget->render(orchestrator, renderTree, parameters);
    lastDepthProjMatrix = proj;
    depthDirty = false;
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
        depthDirty = true; // fresh target must be drawn
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

const RenderTerrain::TerrainMesh& RenderTerrain::getDepthMesh(gfx::Context& context) {
    // The instanced depth pass reuses the full terrain mesh; the source PR's coarser
    // depth-only mesh (getDepthMesh/buildMesh) is a separable optimization not pulled here.
    return getMesh(context);
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
        // Match maplibre-gl-js / Mapbox: the terrain surface is opaque 3D geometry
        // drawn with a depth test+write (LEQUAL, ReadWrite), not the earlier
        // depth-off / "2D for now" hack. On tiled GPUs (this device is PowerVR) opaque
        // depth-tested geometry is eligible for hidden-surface removal, so occluded
        // fragments skip the drape sample instead of always running it.
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

#if MLN_RENDER_BACKEND_OPENGL
void RenderTerrain::packDEMArrayLayer(gfx::Context& context, const UnwrappedTileID& id, const DEMData& demData) {
    const auto& imagePtr = demData.getImagePtr();
    if (!imagePtr || imagePtr->size.isEmpty()) {
        return;
    }
    if (!demTextureArray) {
        demTextureArray = std::make_unique<gl::Texture2DArray>(static_cast<gl::Context&>(context));
    }
    // All DEM tiles from one source share a size, so this allocates once and no-ops after.
    demTextureArray->allocate(imagePtr->size, maxDEMArrayLayers);
    if (!demTextureArray->valid()) {
        return;
    }

    uint32_t layer = 0;
    if (const auto it = demArrayLayer.find(id); it != demArrayLayer.end()) {
        layer = it->second; // re-upload into the tile's existing slot
    } else if (!demArrayFreeLayers.empty()) {
        layer = demArrayFreeLayers.back();
        demArrayFreeLayers.pop_back();
        demArrayLayer[id] = layer;
    } else if (demArrayNextLayer < maxDEMArrayLayers) {
        layer = demArrayNextLayer++;
        demArrayLayer[id] = layer;
    } else {
        return; // array full - tile keeps its per-tile texture, just not instanced
    }
    demTextureArray->uploadLayer(layer, imagePtr->data.get());
}

void RenderTerrain::freeDEMArrayLayer(const UnwrappedTileID& id) {
    if (const auto it = demArrayLayer.find(id); it != demArrayLayer.end()) {
        demArrayFreeLayers.push_back(it->second);
        demArrayLayer.erase(it);
    }
}

// Build the single instanced depth drawable covering the current depthInstances: the shared
// depth mesh drawn N times, with a_instance = [0..N-1] selecting each tile's slot in the
// TerrainDepthInstanceUBO array (filled per frame in updateInstancedDepthUBO). No tile id is
// set, so the terrain tweaker skips it; no gfx textures, since the DEM array is bound manually
// in renderDepth. Called only when the tile set changes.
void RenderTerrain::rebuildInstancedDepthDrawable(gfx::Context& context, gfx::ShaderRegistry& shaders) {
    auto* depthLg = static_cast<LayerGroup*>(depthLayerGroup.get());
    if (!depthLg) {
        return;
    }
    depthLg->clearDrawables();
    const std::size_t n = depthInstances.size();
    if (n == 0 || !demTextureArray || !demTextureArray->valid()) {
        return;
    }

    const auto& depthMeshRef = getDepthMesh(context);
    if (depthMeshRef.vertices.empty() || depthMeshRef.indices.empty()) {
        return;
    }
    auto shader = context.getGenericShader(shaders, "TerrainDepthShader");
    if (!shader) {
        return;
    }
    auto builder = context.createDrawableBuilder("terrain-depth-instanced");
    if (!builder) {
        return;
    }
    builder->setShader(shader);
    builder->setRenderPass(RenderPass::Translucent);
    builder->setDepthType(gfx::DepthMaskType::ReadWrite);
    builder->setColorMode(gfx::ColorMode::unblended());
    builder->setEnableDepth(true);
    builder->setIs3D(true);

    std::vector<uint8_t> vtx(depthMeshRef.vertices.size() * sizeof(int16_t));
    std::memcpy(vtx.data(), depthMeshRef.vertices.data(), vtx.size());
    builder->setRawVertices(std::move(vtx), depthMeshRef.vertexCount, gfx::AttributeDataType::Short4);

    SegmentVector segs;
    segs.emplace_back(0, 0, depthMeshRef.vertexCount, depthMeshRef.indexCount);
    std::vector<uint16_t> idx = depthMeshRef.indices;
    builder->setSegments(gfx::Triangles(), std::move(idx), segs.data(), segs.size());

    // Per-instance index attribute (divisor 1). Its element count is the instance count the
    // GL backend draws (drawInstanced uses instanceAttrs->getMinCount()).
    auto instAttrs = context.createVertexAttributeArray();
    if (const auto& a = instAttrs->set(shaders::idTerrainInstanceVertexAttribute)) {
        for (std::size_t i = 0; i < n; ++i) {
            a->set(i, static_cast<float>(i));
        }
    }
    builder->setInstanceAttributes(std::move(instAttrs));

    builder->flush(context);
    auto drawables = builder->clearDrawables();
    if (!drawables.empty()) {
        // Bind the packed DEM array as u_dem_array (slot idTerrainDEMArrayTexture); DrawableGL
        // binds it in bindTextures() with the program active, using the shader sampler location.
        static_cast<gl::DrawableGL&>(*drawables[0])
            .setArrayTexture(demTextureArray.get(), shaders::idTerrainDEMArrayTexture);
        depthLg->addDrawable(std::move(drawables[0]));
    }
}

// Refresh the per-instance UBO array every frame (the transform depends on the camera) and
// bind it + the shared props UBO directly on the instanced drawable, so the terrain tweaker's
// layer-level TerrainDrawableUBO consolidation does not clobber it. Bound at idTerrainDrawableUBO
// as the array the shader indexes by a_instance.
void RenderTerrain::updateInstancedDepthUBO(PaintParameters& parameters) {
    auto* depthLg = static_cast<LayerGroup*>(depthLayerGroup.get());
    const std::size_t n = depthInstances.size();

    if (!depthLg || n == 0) {
        return;
    }
    auto& context = parameters.context;

    // The shader declares the block as a fixed array u_inst[TERRAIN_MAX_INSTANCES]
    // (== maxDepthInstances), so GLES requires the bound buffer/range to be at least
    // that full static size (sizeof(UBO) * maxDepthInstances). Allocate the whole block
    // and fill only the first n entries; the rest stay zero-initialized. Sizing the
    // buffer to n instead triggers "Bound buffer is too small" and the draw is dropped.
    std::vector<shaders::TerrainDepthInstanceUBO> arr(maxDepthInstances);
    for (std::size_t i = 0; i < n; ++i) {
        const auto& inst = depthInstances[i];
        mat4 m = parameters.matrixForTile(inst.tileID.toUnwrapped());
#if !MLN_RENDER_BACKEND_OPENGL
        m[2] = 0.5 * (m[2] + m[3]);
        m[6] = 0.5 * (m[6] + m[7]);
        m[10] = 0.5 * (m[10] + m[11]);
        m[14] = 0.5 * (m[14] + m[15]);
#endif
        arr[i].matrix = util::cast<float>(m);
        arr[i].dem_coords = inst.demCoords;
        arr[i].dem_layer = inst.demLayer;
        arr[i].pad1 = arr[i].pad2 = arr[i].pad3 = 0.0f;
    }
    const std::size_t bytes = sizeof(shaders::TerrainDepthInstanceUBO) * maxDepthInstances;
    if (!depthInstanceUBO || depthInstanceUBO->getSize() < bytes) {
        depthInstanceUBO = context.createUniformBuffer(arr.data(), bytes, false, true);
    } else {
        depthInstanceUBO->update(arr.data(), bytes);
    }

    // Shared evaluated props (unpack / exaggeration / skirt offset), same as the tweaker.
    const auto zoom = std::max(static_cast<double>(parameters.state.getZoom()), 0.0);
    const float elevationOffset = static_cast<float>(util::M2PI * util::EARTH_RADIUS_M / std::pow(2.0, zoom) / 5.0);
    const shaders::TerrainEvaluatedPropsUBO propsUBO = {.unpack = getDEMUnpackVector(),
                                                        .exaggeration = getExaggeration(),
                                                        .elevation_offset = elevationOffset,
                                                        .pad1 = 0.0f,
                                                        .pad2 = 0.0f};

    depthLg->visitDrawables([&](gfx::Drawable& drawable) {
        auto& u = drawable.mutableUniformBuffers();
        u.set(shaders::idTerrainDrawableUBO, depthInstanceUBO);
        u.createOrUpdate(shaders::idTerrainEvaluatedPropsUBO, &propsUBO, context);
    });
}
#endif

} // namespace mbgl
