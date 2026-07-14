#pragma once

#include <mbgl/style/terrain_impl.hpp>
#include <mbgl/util/immutable.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/renderer/texture_pool.hpp>

#include <array>
#include <memory>
#include <map>
#include <string>
#include <optional>
#include <vector>
#include <cstdint>
#include <unordered_map>

namespace mbgl {

class TransformState;
class UpdateParameters;
class RenderSource;
class PaintParameters;
class RenderTree;
class LayerGroupBase;
class TerrainLayerTweaker;
class DEMData;
using LayerGroupBasePtr = std::shared_ptr<LayerGroupBase>;
using UniqueChangeRequestVec = std::vector<std::unique_ptr<class ChangeRequest>>;

namespace gfx {
class Context;
class Drawable;
class ShaderRegistry;
class Texture2D;
} // namespace gfx

/**
 * @brief Manages 3D terrain rendering using DEM (Digital Elevation Model) data
 *
 * RenderTerrain is responsible for:
 * - Loading and caching DEM tiles from raster-dem sources
 * - Generating and caching terrain mesh geometry
 * - Providing elevation lookups for any coordinate
 * - Managing GPU resources for terrain rendering
 */
class RenderTerrain {
public:
    RenderTerrain(Immutable<style::Terrain::Impl>);
    ~RenderTerrain();

    /**
     * @brief Update terrain state for the current frame
     * @param parameters Update parameters including transform state and sources
     */
    void update(const UpdateParameters& parameters);

    /**
     * @brief Update terrain rendering (create/update drawables)
     * @param orchestrator Render orchestrator for accessing render sources
     * @param shaders Shader registry for getting terrain shader
     * @param context Graphics context for creating drawables and layer groups
     * @param state Transform state
     * @param updateParameters Update parameters
     * @param renderTree Render tree
     * @param changes Vector to collect change requests
     */
    void update(class RenderOrchestrator& orchestrator,
                gfx::ShaderRegistry& shaders,
                gfx::Context& context,
                const TexturePool& texturePool,
                const TransformState& state,
                const std::shared_ptr<UpdateParameters>& updateParameters,
                const RenderTree& renderTree,
                UniqueChangeRequestVec& changes);

    /**
     * @brief Get elevation at a specific tile coordinate
     * @param tileID The tile containing the coordinate
     * @param x X coordinate within the tile
     * @param y Y coordinate within the tile
     * @return Elevation in meters (or 0 if no DEM data available)
     */
    float getElevation(const UnwrappedTileID& tileID, float x, float y) const;

    /**
     * @brief Get elevation with exaggeration applied
     * @param tileID The tile containing the coordinate
     * @param x X coordinate within the tile
     * @param y Y coordinate within the tile
     * @return Elevation in meters with exaggeration multiplier applied
     */
    float getElevationWithExaggeration(const UnwrappedTileID& tileID, float x, float y) const;

    /**
     * @brief Get the terrain exaggeration multiplier
     */
    float getExaggeration() const;

    /**
     * @brief Get the source ID providing DEM data
     */
    const std::string& getSourceID() const;

    /**
     * @brief Check if terrain is enabled and has DEM data
     */
    bool isEnabled() const;

    /**
     * @brief Get the DEM unpack vector for the source's raster-dem encoding
     */
    const std::array<float, 4>& getDEMUnpackVector() const { return demUnpackVector; }

    /**
     * @brief Scale/offset mapping a terrain drawable's uv into its bound DEM
     * texture ({1,0,0,0} unless an ancestor tile's DEM is bound as a fallback)
     */
    std::array<float, 4> getDrawableDemCoords(const OverscaledTileID& tileID) const {
        const auto it = drawableDemCoords.find(tileID);
        return it != drawableDemCoords.end() ? it->second : std::array<float, 4>{{1, 0, 0, 0}};
    }

    /**
     * @brief Per-tile DEM binding data for layers that sample elevation in their
     * vertex shaders (the native analog of maplibre-gl-js terrain.getTerrainData)
     */
    struct TerrainData {
        std::shared_ptr<gfx::Texture2D> demTexture;
        /// scale and x/y offset mapping tile-local coordinates (0..EXTENT) of the
        /// requested tile into normalized coordinates (0..1) of the DEM tile
        std::array<float, 4> demCoords;
        float demDim;
    };

    /**
     * @brief Get the DEM texture and coordinate mapping covering the given tile,
     * from the matching DEM tile or its closest available ancestor
     */
    std::optional<TerrainData> getTerrainData(const UnwrappedTileID&) const;

    /**
     * @brief A 1x1 zero-elevation DEM texture bound when no DEM tile is available,
     * so shaders that declare the DEM sampler always have a valid binding
     */
    const std::shared_ptr<gfx::Texture2D>& getPlaceholderDEMTexture(gfx::Context&);

    /**
     * @brief Get the terrain implementation
     */
    const Immutable<style::Terrain::Impl>& getImpl() const { return impl; }

    /**
     * @brief Get the terrain mesh for a specific tile
     *
     * Returns a cached mesh or generates a new one. The mesh is a regular grid
     * that will be displaced by DEM data in the vertex shader.
     *
     * @param tileID The tile ID (unused currently - same mesh for all tiles)
     * @return Pointer to vertex buffer and index buffer
     */
    struct TerrainMesh {
        std::shared_ptr<gfx::VertexBuffer<float>> vertexBuffer;
        std::shared_ptr<gfx::IndexBuffer> indexBuffer;
        size_t vertexCount;
        size_t indexCount;
        std::vector<int16_t> vertices; // Raw vertex data (x, y pairs as short2)
        std::vector<uint16_t> indices; // Raw index data
    };

    const TerrainMesh& getMesh(gfx::Context& context);

    /**
     * @brief Get the layer group for terrain drawables
     */
    const LayerGroupBasePtr& getLayerGroup() const { return layerGroup; }

    /**
     * @brief Get the terrain layer tweaker
     */
    TerrainLayerTweaker* getTweaker() const { return tweaker.get(); }

    // Immutable terrain configuration
    Immutable<style::Terrain::Impl> impl;

private:
    /**
     * @brief Generate terrain mesh geometry
     *
     * Creates a regular grid mesh (default 128x128) with border frames
     * to prevent stitching artifacts between tiles.
     */
    void generateMesh(gfx::Context& context);

    /**
     * @brief Find the DEM source for the current terrain
     */
    RenderSource* findDEMSource(const UpdateParameters& parameters);

    /**
     * @brief Activate or deactivate the layer group
     */
    void activateLayerGroup(bool activate, UniqueChangeRequestVec& changes);

    // Terrain mesh (shared across all tiles)
    std::optional<TerrainMesh> mesh;

    // Layer group for terrain drawables
    LayerGroupBasePtr layerGroup;

    // Terrain layer tweaker for UBO updates
    std::unique_ptr<TerrainLayerTweaker> tweaker;

    // Track which tiles have terrain drawables; the value is true when the
    // drawable samples the tile's own DEM texture, false when it is using an
    // ancestor tile's DEM as a fallback while its own DEM is still loading
    std::unordered_map<OverscaledTileID, bool> tilesWithDrawables;

    // Per-drawable scale/offset into the bound DEM texture ({1,0,0,0} unless
    // an ancestor tile's DEM is bound); read by the terrain layer tweaker
    std::map<OverscaledTileID, std::array<float, 4>> drawableDemCoords;

    // Mesh resolution (vertices per side)
    static constexpr size_t MESH_SIZE = 128;

    // Cached DEM source
    RenderSource* demSource = nullptr;

    // DEM decode vector for the source's encoding (default: Mapbox Terrain-RGB)
    std::array<float, 4> demUnpackVector = {{6553.6f, 25.6f, 0.1f, 10000.0f}};

    // DEM textures by tile, for elevation sampling by non-draped layers
    struct DEMTextureEntry {
        std::shared_ptr<gfx::Texture2D> texture;
        int32_t dim;
    };
    std::map<UnwrappedTileID, DEMTextureEntry> demTextures;

    // See getPlaceholderDEMTexture
    std::shared_ptr<gfx::Texture2D> placeholderDEMTexture;

    // Layer index (terrain renders early in 3D pass, use negative index)
    static constexpr int32_t TERRAIN_LAYER_INDEX = -1000;

    /**
     * @brief Create a DEM texture from DEMData
     * @param context Graphics context
     * @param demData DEM elevation data
     * @return Shared pointer to created texture
     */
    std::shared_ptr<gfx::Texture2D> createDEMTexture(gfx::Context& context, const DEMData& demData);

    /**
     * @brief Create a test map texture (checkerboard pattern)
     * This will be replaced with render-to-texture output later
     * @param context Graphics context
     * @return Shared pointer to created texture
     */
    std::shared_ptr<gfx::Texture2D> createTestMapTexture(gfx::Context& context);

    /**
     * @brief Create a terrain drawable for a specific tile
     * @param context Graphics context
     * @param shaders Shader registry
     * @param tileID Tile ID for this drawable
     * @param demTexture DEM texture for elevation data
     * @return Unique pointer to created drawable
     */
    std::unique_ptr<gfx::Drawable> createDrawableForTile(gfx::Context& context,
                                                         gfx::ShaderRegistry& shaders,
                                                         const OverscaledTileID& tileID,
                                                         std::shared_ptr<gfx::Texture2D> demTexture,
                                                         std::shared_ptr<gfx::Texture2D> mapTexture);
};

} // namespace mbgl
