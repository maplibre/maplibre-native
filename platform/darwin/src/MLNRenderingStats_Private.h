#import "MLNRenderingStats.h"

#import <mbgl/gfx/rendering_stats.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface MLNOverscaledTileID (Private)
- (instancetype)initWithTileID:(const mbgl::OverscaledTileID &)tileID;
@end

@interface MLNSourceLayerID (Private)
- (instancetype)initWithID:(const mbgl::gfx::RenderingStats::SourceLayerID &)id_;
- (id)copyWithZone:(NSZone *)zone;
@end

@interface MLNFeatureInfo (Private)
- (instancetype)initWithFeatureId:(const std::string &)id_
                      FeatureInfo:(const mbgl::gfx::RenderingStats::FeatureInfo &)info;
@end

@interface MLNRenderingStats (Private)
- (void)setCoreData:(const mbgl::gfx::RenderingStats &)stats;
- (void)setFeatureInfo:(const mbgl::gfx::RenderingStats &)stats;
@end

NS_ASSUME_NONNULL_END
