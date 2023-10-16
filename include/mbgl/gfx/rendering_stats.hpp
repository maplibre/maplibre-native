#pragma once

#include <cstddef>

namespace mbgl {
namespace gfx {

struct RenderingStats {
    RenderingStats() = default;
    bool isZero() const;

    int numFrames = 0;
    int numDrawCalls = 0;
    int totalDrawCalls = 0;

    int numCreatedTextures = 0;
    int numActiveTextures = 0;
    int numTextureBindings = 0;
    int numTextureUpdates = 0;
    std::size_t textureUpdateBytes = 0;

    int numBuffers = 0;
    int numFrameBuffers = 0;

    int numIndexBuffers = 0;
    std::size_t indexUpdateBytes = 0;

    int numVertexBuffers = 0;
    std::size_t vertexUpdateBytes = 0;

    int numUniformBuffers = 0;
    int numUniformUpdates = 0;
    std::size_t uniformUpdateBytes = 0;

    int memTextures = 0;
    int memBuffers = 0;
    int memIndexBuffers = 0;
    int memVertexBuffers = 0;
    int memUniformBuffers = 0;

    int stencilClears = 0;
    int stencilUpdates = 0;

    RenderingStats& operator+=(const RenderingStats&);
};

} // namespace gfx
} // namespace mbgl
