#pragma once

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/shaders/shader_defines.hpp>

#include <memory>
#include <string>

namespace mbgl {

class RenderTerrain;
class LayerGroupBase;
class PaintParameters;

namespace gfx {
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

/**
 * Terrain layer specific tweaker - updates UBOs for terrain rendering
 * Note: This is NOT a LayerTweaker because terrain is not a regular layer
 */
class TerrainLayerTweaker : util::noncopyable {
public:
    explicit TerrainLayerTweaker(const RenderTerrain* terrain_)
        : terrain(terrain_) {}

    ~TerrainLayerTweaker() = default;

    void execute(LayerGroupBase&, const PaintParameters&);

protected:
#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif

    const RenderTerrain* terrain = nullptr;
};

} // namespace mbgl
