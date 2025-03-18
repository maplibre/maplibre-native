#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MLNRenderingStats : NSObject

    /// Frame CPU encoding time (milliseconds)
    @property (readonly) double encodingTime;
    /// Frame CPU rendering time (milliseconds)
    @property (readonly) double renderingTime;

    /// Number of frames rendered
    @property (readonly) int numFrames;
    /// Number of draw calls (`glDrawElements`, `drawIndexedPrimitives`, etc.) executed during the most recent frame
    @property (readonly) int numDrawCalls;
    /// Total number of draw calls executed during all the frames
    @property (readonly) int totalDrawCalls;

    /// Total number of textures created
    @property (readonly) int numCreatedTextures;
    /// Net textures
    @property (readonly) int numActiveTextures;
    /// Net texture bindings
    @property (readonly) int numTextureBindings;
    /// Number of times a texture was updated
    @property (readonly) int numTextureUpdates;
    /// Number of bytes used in texture updates
    @property (readonly) unsigned long textureUpdateBytes;

    /// Number of buffers created
    @property (readonly) unsigned long totalBuffers;
    /// Number of SDK-specific buffers created
    @property (readonly) unsigned long totalBufferObjs;
    /// Number of times a buffer is updated
    @property (readonly) unsigned long bufferUpdates;
    /// Number of times an SDK-specific buffer is updated
    @property (readonly) unsigned long bufferObjUpdates;
    /// Sum of update sizes
    @property (readonly) unsigned long bufferUpdateBytes;

    /// Number of active buffers
    @property (readonly) int numBuffers;
    @property (readonly) int numFrameBuffers;

    @property (readonly) int numIndexBuffers;
    @property (readonly) unsigned long indexUpdateBytes;

    @property (readonly) int numVertexBuffers;
    @property (readonly) unsigned long vertexUpdateBytes;

    @property (readonly) int numUniformBuffers;
    @property (readonly) int numUniformUpdates;
    @property (readonly) unsigned long uniformUpdateBytes;

    @property (readonly) int memTextures;
    @property (readonly) int memBuffers;
    @property (readonly) int memIndexBuffers;
    @property (readonly) int memVertexBuffers;
    @property (readonly) int memUniformBuffers;

    @property (readonly) int stencilClears;
    @property (readonly) int stencilUpdates;
@end

NS_ASSUME_NONNULL_END
