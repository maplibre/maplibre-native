#import "MLNRenderingStats_Private.h"

#import <CoreGraphics/CoreGraphics.h>

@implementation MLNOverscaledTileID
- (instancetype)initWithTileID:(const mbgl::OverscaledTileID&)tileID {
  if (self = [super init]) {
    _tileZ = tileID.canonical.z;
    _tileX = tileID.canonical.x;
    _tileY = tileID.canonical.y;
    _overscaledZ = tileID.overscaledZ;
    _wrap = tileID.wrap;
  }
  return self;
}
@end

@implementation MLNSourceLayerID
- (instancetype)initWithID:(const mbgl::gfx::RenderingStats::SourceLayerID&)id_ {
  if (self = [super init]) {
    _sourceID = [NSString stringWithUTF8String:id_.sourceID.c_str()];
    _layerID = [NSString stringWithUTF8String:id_.layerID.c_str()];
  }
  return self;
}
- (id)copyWithZone:(NSZone*)zone {
  MLNSourceLayerID* copy = [[[self class] allocWithZone:zone] init];
  if (copy) {
    copy->_sourceID = [_sourceID copyWithZone:zone];
    copy->_layerID = [_layerID copyWithZone:zone];
  }
  return copy;
}
@end

@implementation MLNFeatureInfo
- (instancetype)initWithFeatureId:(const std::string&)id_
                      FeatureInfo:(const mbgl::gfx::RenderingStats::FeatureInfo&)info {
  if (self = [super init]) {
    _featureID = [NSString stringWithUTF8String:id_.c_str()];
    _ndcBound = CGRectMake(static_cast<CGFloat>(info.ndcBound.minX),
                           static_cast<CGFloat>(info.ndcBound.minY),
                           static_cast<CGFloat>(info.ndcBound.maxX - info.ndcBound.minX),
                           static_cast<CGFloat>(info.ndcBound.maxY - info.ndcBound.minY));
    NSMutableSet<MLNOverscaledTileID*>* tileIDs =
        [NSMutableSet setWithCapacity:info.tileIDs.size()];
    for (const auto& tileID : info.tileIDs) {
      [tileIDs addObject:[[MLNOverscaledTileID alloc] initWithTileID:tileID]];
    }
    _tileIDs = tileIDs;
  }
  return self;
}
@end

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

- (void)setFeatureInfo:(const mbgl::gfx::RenderingStats&)stats {
  NSMutableDictionary<MLNSourceLayerID*, NSArray<MLNFeatureInfo*>*>* frameRenderedFeatures =
      [NSMutableDictionary dictionary];
  for (const auto& [sourceLayerID, layerFeatures] : stats.frameRenderedFeatures) {
    MLNSourceLayerID* featuresKey = [[MLNSourceLayerID alloc] initWithID:sourceLayerID];
    NSMutableArray* featuresArray = [NSMutableArray arrayWithCapacity:layerFeatures.size()];
    for (const auto& [featureID, featureInfo] : layerFeatures) {
      [featuresArray addObject:[[MLNFeatureInfo alloc] initWithFeatureId:featureID
                                                             FeatureInfo:featureInfo]];
    }
    [frameRenderedFeatures setObject:featuresArray forKey:featuresKey];
  }
  _frameRenderedFeatures = frameRenderedFeatures;
}

@end
