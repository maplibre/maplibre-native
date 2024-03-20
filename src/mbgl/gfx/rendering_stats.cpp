#include <mbgl/gfx/rendering_stats.hpp>

#include <algorithm>
#include <initializer_list>
#include <sstream>

namespace mbgl {
namespace gfx {

bool RenderingStats::isZero() const {
    const auto expectedZeros = {numActiveTextures,
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
                                memUniformBuffers};
    return std::all_of(expectedZeros.begin(), expectedZeros.end(), [](auto x) { return x == 0; });
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
    totalBuffers += r.totalBuffers;
    totalBufferObjs += r.totalBufferObjs;
    bufferUpdates += r.bufferUpdates;
    bufferObjUpdates += r.bufferObjUpdates;
    bufferUpdateBytes += r.bufferUpdateBytes;
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

#if !defined(NDEBUG)
std::string RenderingStats::toString(std::string_view sep) const {
    std::stringstream ss;
    ss << "numFrames = " << numFrames << sep << "numDrawCalls = " << numDrawCalls << sep
       << "totalDrawCalls = " << totalDrawCalls << sep << "numCreatedTextures = " << numCreatedTextures << sep
       << "numActiveTextures = " << numActiveTextures << sep << "numTextureBindings = " << numTextureBindings << sep
       << "numTextureUpdates = " << numTextureUpdates << sep << "textureUpdateBytes = " << textureUpdateBytes << sep
       << "totalBuffers = " << totalBuffers << sep << "totalBufferObjs = " << totalBufferObjs << sep
       << "bufferUpdates = " << bufferUpdates << sep << "bufferObjUpdates = " << bufferObjUpdates << sep
       << "bufferUpdateBytes = " << bufferUpdateBytes << sep << "numBuffers = " << numBuffers << sep
       << "numFrameBuffers = " << numFrameBuffers << sep << "numIndexBuffers = " << numIndexBuffers << sep
       << "indexUpdateBytes = " << indexUpdateBytes << sep << "numVertexBuffers = " << numVertexBuffers << sep
       << "vertexUpdateBytes = " << vertexUpdateBytes << sep << "numUniformBuffers = " << numUniformBuffers << sep
       << "numUniformUpdates = " << numUniformUpdates << sep << "uniformUpdateBytes = " << uniformUpdateBytes << sep
       << "memTextures = " << memTextures << sep << "memBuffers = " << memBuffers << sep
       << "memIndexBuffers = " << memIndexBuffers << sep << "memVertexBuffers = " << memVertexBuffers << sep
       << "memUniformBuffers = " << memUniformBuffers << sep << "stencilClears = " << stencilClears << sep
       << "stencilUpdates = " << stencilUpdates << sep;
    return ss.str();
}
#endif

} // namespace gfx
} // namespace mbgl
