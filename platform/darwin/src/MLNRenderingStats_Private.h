#import "MLNRenderingStats.h"
#import <mbgl/gfx/rendering_stats.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface MLNRenderingStats (Private)

- (void)setCoreData:(const mbgl::gfx::RenderingStats&)stats;

@end

NS_ASSUME_NONNULL_END
