#pragma once

#include <mln/util/noncopyable.hpp>
#include <mln/shaders/shader_defines.hpp>

#include <memory>
#include <string>

namespace mln {

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
    // One buffer serves both layer groups, the terrain group and the depth group: each execute()
    // rewrites it, so the group that runs last decides what the buffer holds, and the other group's
    // drawables read it by their own position. A group lists its drawables in creation order, so the
    // two agree only while a tile's terrain drawable and depth drawable are made and dropped together,
    // as RenderTerrain::update does (and it starts both groups over should they ever differ).
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif

    const RenderTerrain* terrain = nullptr;
};

} // namespace mln
