#import <Foundation/Foundation.h>
#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

MLN_EXPORT
@interface MLNRenderingStats : NSObject

/// Frame CPU encoding time (seconds)
@property (readonly) double encodingTime;
/// Frame CPU rendering time (seconds)
@property (readonly) double renderingTime;

/// Number of frames rendered
@property (readonly) int numFrames;
/// Number of draw calls (`glDrawElements`, `drawIndexedPrimitives`, etc.) executed during the most
/// recent frame
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
/// Number of active offscreen frame buffers
@property (readonly) int numFrameBuffers;

/// Number of active index buffers
@property (readonly) int numIndexBuffers;
/// Sum of index buffers update sizes
@property (readonly) unsigned long indexUpdateBytes;

/// Number of active vertex buffers
@property (readonly) int numVertexBuffers;
/// Sum of vertex buffers update sizes
@property (readonly) unsigned long vertexUpdateBytes;

/// Number of active uniform buffers
@property (readonly) int numUniformBuffers;
/// Number of times a uniform buffer is updated
@property (readonly) int numUniformUpdates;
/// Sum of uniform buffers update sizes
@property (readonly) unsigned long uniformUpdateBytes;

/// Total texture memory
@property (readonly) int memTextures;
/// Total buffer memory
@property (readonly) int memBuffers;
/// Total index buffer memory
@property (readonly) int memIndexBuffers;
/// Total vertex buffer memory
@property (readonly) int memVertexBuffers;
/// Total uniform buffer memory
@property (readonly) int memUniformBuffers;

/// Number of stencil buffer clears
@property (readonly) int stencilClears;
/// Number of stencil buffer updates
@property (readonly) int stencilUpdates;
@end

NS_ASSUME_NONNULL_END
