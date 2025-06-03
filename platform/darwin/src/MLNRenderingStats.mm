#import "MLNRenderingStats_Private.h"

@implementation MLNRenderingStats

- (void)setCoreData:(const mbgl::gfx::RenderingStats&)stats {
    _encodingTime = stats.encodingTime;
    _renderingTime = stats.renderingTime;
    _numFrames = stats.numFrames;
    _numDrawCalls = stats.numDrawCalls;
    _totalDrawCalls = stats.totalDrawCalls;
    _numCreatedTextures = stats.numCreatedTextures;
    _numActiveTextures = stats.numActiveTextures;
    _numTextureBindings = stats.numTextureBindings;
    _numTextureUpdates = stats.numTextureUpdates;
    _textureUpdateBytes = stats.textureUpdateBytes;
    _totalBuffers = stats.totalBuffers;
    _totalBufferObjs = stats.totalBufferObjs;
    _bufferUpdates = stats.bufferUpdates;
    _bufferObjUpdates = stats.bufferObjUpdates;
    _bufferUpdateBytes = stats.bufferUpdateBytes;
    _numIndexBuffers = stats.numIndexBuffers;
    _indexUpdateBytes = stats.indexUpdateBytes;
    _numVertexBuffers = stats.numVertexBuffers;
    _vertexUpdateBytes = stats.vertexUpdateBytes;
    _numUniformBuffers = stats.numUniformBuffers;
    _numUniformUpdates = stats.numUniformUpdates;
    _uniformUpdateBytes = stats.uniformUpdateBytes;
    _memTextures = stats.memTextures;
    _memBuffers = stats.memBuffers;
    _memIndexBuffers = stats.memIndexBuffers;
    _memVertexBuffers = stats.memVertexBuffers;
    _memUniformBuffers = stats.memUniformBuffers;
    _stencilClears = stats.stencilClears;
    _stencilUpdates = stats.stencilUpdates;
}


@end
