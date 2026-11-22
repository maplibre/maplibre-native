#import <Foundation/Foundation.h>
#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

MLN_EXPORT
@interface MLNOverscaledTileID : NSObject
@property (readonly, nonatomic) int tileX;
@property (readonly, nonatomic) int tileY;
@property (readonly, nonatomic) int tileZ;
@property (readonly, nonatomic) int overscaledZ;
@property (readonly, nonatomic) int wrap;
@end

MLN_EXPORT
@interface MLNSourceLayerID : NSObject <NSCopying>
@property (readonly, nonatomic) NSString *sourceID;
@property (readonly, nonatomic) NSString *layerID;
@end

MLN_EXPORT
@interface MLNFeatureInfo : NSObject
@property (readonly, nonatomic) NSString *featureID;
/// The bounding box for this feature in NDC coordinates
/// NDC positive is up/right
@property (readonly, nonatomic) CGRect ndcBound;
@property (readonly, nonatomic) NSSet<MLNOverscaledTileID *> *tileIDs;
@end

MLN_EXPORT
@interface MLNRenderingStats : NSObject

/// Frame CPU encoding time (seconds)
@property (readonly, nonatomic) double encodingTime;
/// Frame CPU rendering time (seconds)
@property (readonly, nonatomic) double renderingTime;

/// Number of frames rendered
@property (readonly, nonatomic) int numFrames;
/// Number of draw calls (`glDrawElements`, `drawIndexedPrimitives`, etc.) executed during the most
/// recent frame
@property (readonly, nonatomic) int numDrawCalls;
/// Total number of draw calls executed during all the frames
@property (readonly, nonatomic) int totalDrawCalls;

/// Total number of textures created
@property (readonly, nonatomic) int numCreatedTextures;
/// Net textures
@property (readonly, nonatomic) int numActiveTextures;
/// Net texture bindings
@property (readonly, nonatomic) int numTextureBindings;
/// Number of times a texture was updated
@property (readonly, nonatomic) int numTextureUpdates;
/// Number of bytes used in texture updates
@property (readonly, nonatomic) unsigned long textureUpdateBytes;

/// Number of buffers created
@property (readonly, nonatomic) unsigned long totalBuffers;
/// Number of SDK-specific buffers created
@property (readonly, nonatomic) unsigned long totalBufferObjs;
/// Number of times a buffer is updated
@property (readonly, nonatomic) unsigned long bufferUpdates;
/// Number of times an SDK-specific buffer is updated
@property (readonly, nonatomic) unsigned long bufferObjUpdates;
/// Sum of update sizes
@property (readonly, nonatomic) unsigned long bufferUpdateBytes;

/// Number of active buffers
@property (readonly, nonatomic) int numBuffers;
/// Number of active offscreen frame buffers
@property (readonly, nonatomic) int numFrameBuffers;

/// Number of active index buffers
@property (readonly, nonatomic) int numIndexBuffers;
/// Sum of index buffers update sizes
@property (readonly, nonatomic) unsigned long indexUpdateBytes;

/// Number of active vertex buffers
@property (readonly, nonatomic) int numVertexBuffers;
/// Sum of vertex buffers update sizes
@property (readonly, nonatomic) unsigned long vertexUpdateBytes;

/// Number of active uniform buffers
@property (readonly, nonatomic) int numUniformBuffers;
/// Number of times a uniform buffer is updated
@property (readonly, nonatomic) int numUniformUpdates;
/// Sum of uniform buffers update sizes
@property (readonly, nonatomic) unsigned long uniformUpdateBytes;

/// Total texture memory
@property (readonly, nonatomic) int memTextures;
/// Total buffer memory
@property (readonly, nonatomic) int memBuffers;
/// Total index buffer memory
@property (readonly, nonatomic) int memIndexBuffers;
/// Total vertex buffer memory
@property (readonly, nonatomic) int memVertexBuffers;
/// Total uniform buffer memory
@property (readonly, nonatomic) int memUniformBuffers;

/// Number of stencil buffer clears
@property (readonly, nonatomic) int stencilClears;
/// Number of stencil buffer updates
@property (readonly, nonatomic) int stencilUpdates;

/* Collected feature information by layer, if enabled via MLNMapOptions.featureInfoEnabled
    (MLNSourceLayerID -> [MLNFeatureInfo])
 */
@property (readonly, nonatomic) NSDictionary *frameRenderedFeatures;

@end

NS_ASSUME_NONNULL_END
