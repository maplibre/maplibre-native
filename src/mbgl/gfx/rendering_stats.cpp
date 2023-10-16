#include <mbgl/gfx/rendering_stats.hpp>

#include <initializer_list>

namespace mbgl {
namespace gfx {

bool RenderingStats::isZero() const {
    for (const auto& n : {numActiveTextures,
                          numTextureBindings,
                          numBuffers,
                          numVertexBuffers,
                          numIndexBuffers,
                          numUniformBuffers,
                          numFrameBuffers,
                          memTextures,
                          memBuffers,
                          memIndexBuffers,
                          memVertexBuffers,
                          memUniformBuffers}) {
        if (n != 0) {
            return false;
        }
    }
    return true;
}

RenderingStats& RenderingStats::operator+=(const RenderingStats& r) {
    numFrames += r.numFrames;
    numDrawCalls += r.numDrawCalls;
    totalDrawCalls += r.totalDrawCalls;
    numCreatedTextures += r.numCreatedTextures;
    numActiveTextures += r.numActiveTextures;
    numTextureBindings += r.numTextureBindings;
    numTextureUpdates += r.numTextureUpdates;
    textureUpdateBytes += r.textureUpdateBytes;
    numBuffers += r.numBuffers;
    numFrameBuffers += r.numFrameBuffers;
    numIndexBuffers += r.numIndexBuffers;
    indexUpdateBytes += r.indexUpdateBytes;
    numVertexBuffers += r.numVertexBuffers;
    vertexUpdateBytes += r.vertexUpdateBytes;
    numUniformBuffers += r.numUniformBuffers;
    numUniformUpdates += r.numUniformUpdates;
    uniformUpdateBytes += r.uniformUpdateBytes;
    memTextures += r.memTextures;
    memBuffers += r.memBuffers;
    memIndexBuffers += r.memIndexBuffers;
    memVertexBuffers += r.memVertexBuffers;
    memUniformBuffers += r.memUniformBuffers;
    stencilClears += r.stencilClears;
    stencilUpdates += r.stencilUpdates;
    return *this;
}

} // namespace gfx
} // namespace mbgl
