#include <mbgl/renderer/render_terrain.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/terrain_layer_tweaker.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/geometry/dem_data.hpp>
#include <mbgl/tile/raster_dem_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/gfx/context.hpp>
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

#include <cmath>
#include <cstring>

namespace mbgl {

RenderTerrain::RenderTerrain(Immutable<style::Terrain::Impl> impl_)
    : impl(std::move(impl_)) {}

RenderTerrain::~RenderTerrain() = default;

void RenderTerrain::update(const UpdateParameters& /*parameters*/) {
    // Find the DEM source if we haven't already
    if (!demSource && !impl->sourceID.empty()) {
        // In a full implementation, we would look up the source from parameters.sources
        // and cache the RenderSource pointer
        // For now, this is a placeholder
    }
}

void RenderTerrain::update(RenderOrchestrator& orchestrator,
                           gfx::ShaderRegistry& shaders,
                           gfx::Context& context,
                           const TexturePool& texturePool,
                           const TransformState& /*state*/,
                           const std::shared_ptr<UpdateParameters>& /*updateParameters*/,
                           const RenderTree& /*renderTree*/,
                           UniqueChangeRequestVec& changes) {
    // Find the DEM source if we haven't already
    if (!demSource && !impl->sourceID.empty()) {
        demSource = orchestrator.getRenderSource(impl->sourceID);
        if (demSource) {
            Log::Info(Event::Render, "Terrain found DEM source: " + impl->sourceID);
        } else {
            Log::Warning(Event::Render, "Terrain could not find DEM source: " + impl->sourceID);
        }
    }

    // TEMP: Always rebuild to pick up latest code changes
    // Clear and recreate layer group every time until terrain is stable
    if (layerGroup && tilesWithDrawables.size() > 0) {
        Log::Info(Event::Render,
                  "Force rebuilding terrain layer group (had " + std::to_string(tilesWithDrawables.size()) +
                      " old drawables)");
        activateLayerGroup(false, changes); // Deactivate old layer group
        layerGroup.reset();                 // Clear the layer group to force recreation
        tilesWithDrawables.clear();
    }

    // Create layer group if we don't have one (including after rebuild)
    if (!layerGroup) {
        if (auto layerGroup_ = context.createLayerGroup(TERRAIN_LAYER_INDEX, /*initialCapacity=*/1, "terrain", false)) {
            layerGroup = std::move(layerGroup_);
            activateLayerGroup(true, changes);
            Log::Info(Event::Render, "Created terrain layer group");
        } else {
            Log::Error(Event::Render, "Failed to create terrain layer group");
            return;
        }
    }

    // Create tweaker if we don't have one
    if (!tweaker) {
        tweaker = std::make_unique<TerrainLayerTweaker>(this);
        Log::Info(Event::Render, "Created terrain layer tweaker");
    }

    // If we don't have a DEM source, we can't create terrain drawables
    if (!demSource) {
        return;
    }

    // Get tiles from the DEM source
    auto renderTiles = demSource->getRawRenderTiles();
    if (renderTiles->empty()) {
        Log::Warning(Event::Render, "Terrain DEM source has no tiles loaded yet");
        return;
    }

    Log::Info(Event::Render, "Terrain processing " + std::to_string(renderTiles->size()) + " DEM tiles");

    // Cast to LayerGroup for addDrawable
    auto* lg = static_cast<LayerGroup*>(layerGroup.get());
    if (!lg) {
        return;
    }

    // Create terrain drawables for each DEM tile
    size_t newDrawables = 0;
    for (const auto& renderTile : *renderTiles) {
        const auto& tileID = renderTile.getOverscaledTileID();

        Log::Info(Event::Render, "Terrain examining tile " + util::toString(tileID));

        // Skip if we already have a drawable for this tile
        if (tilesWithDrawables.count(tileID) > 0) {
            Log::Info(Event::Render, "Terrain tile " + util::toString(tileID) + " already has drawable, skipping");
            continue;
        }

        // Get the underlying Tile and cast to RasterDEMTile
        const auto& tile = renderTile.getTile();
        Log::Info(Event::Render,
                  "Terrain tile " + util::toString(tileID) + " kind=" + std::to_string(static_cast<int>(tile.kind)));

        if (tile.kind != Tile::Kind::RasterDEM) {
            Log::Warning(Event::Render, "Terrain tile " + util::toString(tileID) + " is not RasterDEM type");
            continue;
        }

        auto* demTile = const_cast<RasterDEMTile*>(static_cast<const RasterDEMTile*>(&tile));
        if (!demTile) {
            continue;
        }

        // Get the HillshadeBucket from the DEM tile
        auto* hillshadeBucket = demTile->getBucket();
        if (!hillshadeBucket) {
            Log::Info(Event::Render, "Terrain tile " + util::toString(tileID) + " has no bucket yet (still loading)");
            continue;
        }

        // Get DEM data from the bucket
        const auto& demData = hillshadeBucket->getDEMData();
        auto imagePtr = demData.getImagePtr();
        if (!imagePtr || imagePtr->size.isEmpty()) {
            Log::Warning(Event::Render, "Terrain tile " + util::toString(tileID) + " has empty DEM data");
            continue;
        }

        Log::Info(Event::Render,
                  "Terrain creating texture for tile " + util::toString(tileID) + " (DEM size: " +
                      std::to_string(imagePtr->size.width) + "x" + std::to_string(imagePtr->size.height) + ")");

        // Create DEM texture from the data
        auto demTexture = createDEMTexture(context, demData);
        if (!demTexture) {
            Log::Warning(Event::Render, "Failed to create DEM texture for tile " + util::toString(tileID));
            continue;
        }

        // Create terrain drawable for this tile
        auto drawable = createDrawableForTile(
            context, shaders, tileID, demTexture, texturePool.getRenderTarget(renderTile.id)->getTexture());
        if (drawable) {
            lg->addDrawable(std::move(drawable));
            tilesWithDrawables[tileID] = true;
            newDrawables++;
            Log::Info(Event::Render, "Created terrain drawable for tile " + util::toString(tileID));
        }
    }

    if (newDrawables > 0) {
        Log::Info(Event::Render,
                  "Terrain created " + std::to_string(newDrawables) +
                      " new drawables (total: " + std::to_string(tilesWithDrawables.size()) + ")");
    }
}

float RenderTerrain::getElevation(const UnwrappedTileID& /*tileID*/, float /*x*/, float /*y*/) const {
    // TODO: Implement DEM tile lookup and bilinear interpolation
    // This would:
    // 1. Find the DEM tile covering this coordinate
    // 2. Get the DEMData from the tile
    // 3. Perform bilinear interpolation to get elevation
    return 0.0f;
}

float RenderTerrain::getElevationWithExaggeration(const UnwrappedTileID& tileID, float x, float y) const {
    return getElevation(tileID, x, y) * getExaggeration();
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
    // Generate a regular grid mesh for terrain
    // This mesh will be reused for all tiles and displaced by DEM data in shaders

    const size_t gridSize = MESH_SIZE;
    const size_t verticesPerSide = gridSize + 1;
    const size_t totalVertices = verticesPerSide * verticesPerSide;

    // Vertex data: Each vertex has pos (x,y) and texture_pos (u,v)
    // Store as int16_t (short) for Metal short2 attribute format
    // Format: [pos.x, pos.y, tex.u, tex.v, pos.x, pos.y, tex.u, tex.v, ...]
    std::vector<int16_t> vertices;
    vertices.reserve(totalVertices * 4); // 4 shorts per vertex (x, y, u, v)

    const float posStep = static_cast<float>(util::EXTENT) / static_cast<float>(gridSize);
    const float texStep = static_cast<float>(util::EXTENT) / static_cast<float>(gridSize);

    for (size_t y = 0; y < verticesPerSide; ++y) {
        for (size_t x = 0; x < verticesPerSide; ++x) {
            // Position coordinates (in tile space 0-8192)
            vertices.push_back(static_cast<int16_t>(x * posStep));
            vertices.push_back(static_cast<int16_t>(y * posStep));
            // Texture coordinates (same as position for now - will be used to sample DEM)
            vertices.push_back(static_cast<int16_t>(x * texStep));
            vertices.push_back(static_cast<int16_t>(y * texStep));
        }
    }

    // Index data: generate triangles for the grid
    std::vector<uint16_t> indices;
    indices.reserve(gridSize * gridSize * 6); // 2 triangles per grid cell, 3 indices per triangle

    for (size_t y = 0; y < gridSize; ++y) {
        for (size_t x = 0; x < gridSize; ++x) {
            // Calculate vertex indices for this grid cell
            uint16_t topLeft = static_cast<uint16_t>(y * verticesPerSide + x);
            uint16_t topRight = topLeft + 1;
            uint16_t bottomLeft = static_cast<uint16_t>((y + 1) * verticesPerSide + x);
            uint16_t bottomRight = bottomLeft + 1;

            // First triangle (top-left, bottom-left, top-right)
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle (top-right, bottom-left, bottom-right)
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    // Store mesh data with raw vertices and indices
    mesh = TerrainMesh{nullptr,             // vertexBuffer - will be created when creating drawable
                       nullptr,             // indexBuffer - will be created when creating drawable
                       vertices.size() / 4, // 4 shorts per vertex (x, y, u, v)
                       indices.size(),
                       std::move(vertices),
                       std::move(indices)};

    Log::Info(Event::General,
              "Terrain mesh generated: " + std::to_string(mesh->vertexCount) + " vertices, " +
                  std::to_string(mesh->indexCount) + " indices");
}

std::shared_ptr<gfx::Texture2D> RenderTerrain::createDEMTexture(gfx::Context& context, const DEMData& demData) {
    // Get the DEM image data
    auto imagePtr = demData.getImagePtr();
    if (!imagePtr || imagePtr->size.isEmpty()) {
        Log::Warning(Event::Render, "DEM data has no image");
        return nullptr;
    }

    Log::Info(Event::Render,
              "Creating DEM texture: size=" + std::to_string(imagePtr->size.width) + "x" +
                  std::to_string(imagePtr->size.height) + ", bytes=" + std::to_string(imagePtr->bytes()));

    // DEBUG: Check actual pixel values in the image
    if (imagePtr->data && imagePtr->bytes() > 0) {
        const uint8_t* pixels = imagePtr->data.get();
        // Check first 10 pixels' RGB values
        std::string pixelValues = "First 10 DEM pixels (RGBA): ";
        for (size_t i = 0; i < std::min(size_t(10), imagePtr->bytes() / 4); i++) {
            size_t offset = i * 4;
            pixelValues += "[" + std::to_string(pixels[offset]) + "," + std::to_string(pixels[offset + 1]) + "," +
                           std::to_string(pixels[offset + 2]) + "," + std::to_string(pixels[offset + 3]) + "] ";
        }
        Log::Info(Event::Render, pixelValues);
    }

    // Create a new texture
    auto texture = context.createTexture2D();
    if (!texture) {
        Log::Error(Event::Render, "Failed to create DEM texture");
        return nullptr;
    }

    // Set the image data
    texture->setImage(imagePtr);
    Log::Info(Event::Render, "DEM texture image data set successfully");

    // Configure sampler - use linear filtering for smooth elevation interpolation
    texture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                      .wrapU = gfx::TextureWrapType::Clamp,
                                      .wrapV = gfx::TextureWrapType::Clamp});

    Log::Info(Event::Render, "DEM texture created and configured successfully");
    return texture;
}

std::shared_ptr<gfx::Texture2D> RenderTerrain::createTestMapTexture(gfx::Context& context) {
    // Create a simple test texture with a checkerboard pattern
    // This will be replaced with actual render-to-texture output later
    const uint32_t size = 512;       // 512x512 texture
    const uint32_t checkerSize = 64; // Size of each checker square

    // Create RGBA pixel data
    auto imageData = std::make_unique<uint8_t[]>(size * size * 4);

    for (uint32_t y = 0; y < size; y++) {
        for (uint32_t x = 0; x < size; x++) {
            uint32_t index = (y * size + x) * 4;

            // Create checkerboard pattern
            bool isWhite = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;

            if (isWhite) {
                // White with full alpha
                imageData[index + 0] = 255; // R
                imageData[index + 1] = 255; // G
                imageData[index + 2] = 255; // B
                imageData[index + 3] = 255; // A
            } else {
                // Light blue with full alpha
                imageData[index + 0] = 100; // R
                imageData[index + 1] = 150; // G
                imageData[index + 2] = 255; // B
                imageData[index + 3] = 255; // A
            }
        }
    }

    // Create PremultipliedImage from raw data
    auto image = std::make_shared<PremultipliedImage>(Size{size, size}, std::move(imageData));

    // Create texture
    auto texture = context.createTexture2D();
    if (!texture) {
        Log::Error(Event::Render, "Failed to create test map texture");
        return nullptr;
    }

    // Set the image
    texture->setImage(image);

    // Configure sampler
    texture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                      .wrapU = gfx::TextureWrapType::Repeat,
                                      .wrapV = gfx::TextureWrapType::Repeat});

    Log::Info(Event::Render, "Test map texture created: " + std::to_string(size) + "x" + std::to_string(size));
    return texture;
}

std::unique_ptr<gfx::Drawable> RenderTerrain::createDrawableForTile(gfx::Context& context,
                                                                    gfx::ShaderRegistry& shaders,
                                                                    const OverscaledTileID& tileID,
                                                                    std::shared_ptr<gfx::Texture2D> demTexture,
                                                                    std::shared_ptr<gfx::Texture2D> mapTexture) {
    // Ensure mesh is generated
    const auto& terrainMesh = getMesh(context);

    if (terrainMesh.vertices.empty() || terrainMesh.indices.empty()) {
        Log::Error(Event::Render, "Terrain mesh is empty, cannot create drawable");
        return nullptr;
    }

    // Get terrain shader
    auto terrainShader = context.getGenericShader(shaders, "TerrainShader");
    if (!terrainShader) {
        Log::Error(Event::Render, "Terrain shader not found");
        return nullptr;
    }

    // Create drawable builder
    auto builder = context.createDrawableBuilder("terrain-tile");
    if (!builder) {
        Log::Error(Event::Render, "Failed to create drawable builder for terrain tile");
        return nullptr;
    }

    // Configure builder - terrain is 3D and writes depth
    // NOTE: Using Translucent pass because Opaque pass renders in REVERSE order (high index = back)
    // TEMP: Disable depth testing to render on top of everything
    builder->setShader(terrainShader);
    builder->setRenderPass(RenderPass::Translucent); // Translucent pass renders in forward order (high index = front)
    builder->setDepthType(gfx::DepthMaskType::ReadOnly); // Don't write depth
    builder->setColorMode(gfx::ColorMode::unblended());
    builder->setEnableDepth(false); // Disable depth testing
    builder->setIs3D(false);        // Treat as 2D for now

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
        Log::Info(Event::Render, "DEM texture bound to drawable for tile " + util::toString(tileID));
    } else {
        Log::Warning(Event::Render, "No DEM texture provided for tile " + util::toString(tileID));
    }

    if (!mapTexture) {
        mapTexture = createTestMapTexture(context);
        Log::Warning(Event::Render, "No map texture provided, using test pattern for tile " + util::toString(tileID));
    }
    if (mapTexture) {
        builder->setTexture(mapTexture, 1); // Texture index 1 for map
        Log::Info(Event::Render, "Map texture bound to drawable for tile " + util::toString(tileID));
    } else {
        Log::Warning(Event::Render, "Failed to create test map texture for tile " + util::toString(tileID));
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

RenderSource* RenderTerrain::findDEMSource(const UpdateParameters& /*parameters*/) {
    // TODO: Implement source lookup
    // This would iterate through parameters.sources to find the raster-dem source
    // matching impl->sourceID
    return nullptr;
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

} // namespace mbgl
