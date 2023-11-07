#pragma once

#include <cstddef>
#include <string>

namespace mbgl {
namespace gfx {

struct RenderingStats {
    RenderingStats() = default;
    bool isZero() const;

    /// Number of frames rendered
    int numFrames = 0;
    /// Number of draw calls (`glDrawElements`, `drawIndexedPrimitives`, etc.) executed during the most recent frame
    int numDrawCalls = 0;
    /// Total number of draw calls executed during all the frames
    int totalDrawCalls = 0;

    /// Total number of textures created
    int numCreatedTextures = 0;
    /// Net textures
    int numActiveTextures = 0;
    /// Net texture bindings
    int numTextureBindings = 0;
    /// Number of times a texture was updated
    int numTextureUpdates = 0;
    /// Number of bytes used in texture updates
    std::size_t textureUpdateBytes = 0;

    /// Number of buffers created
    std::size_t totalBuffers = 0;
    /// Number of SDK-specific buffers created
    std::size_t totalBufferObjs = 0;
    /// Number of times a buffer is updated
    std::size_t bufferUpdates = 0;
    /// Number of times an SDK-specific buffer is updated
    std::size_t bufferObjUpdates = 0;
    /// Sum of update sizes
    std::size_t bufferUpdateBytes = 0;

    /// Number of active buffers
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

#if !defined(NDEBUG)
    std::string toString(std::string_view separator) const;
#endif
};

} // namespace gfx
} // namespace mbgl
