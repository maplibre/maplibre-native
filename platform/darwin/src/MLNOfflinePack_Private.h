#import "MLNOfflinePack.h"

#include <mbgl/storage/offline.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface MLNOfflinePack (Private)

@property (nonatomic, nullable) mbgl::OfflineRegion *mbglOfflineRegion;

@property (nonatomic, readwrite) MLNOfflinePackState state;

- (instancetype)initWithMBGLRegion:(mbgl::OfflineRegion *)region;

/**
 Invalidates the pack and ensures that no future progress update can ever
 revalidate it. This method must be called before the pack is deallocated.
 */
- (void)invalidate;

@end

NS_ASSUME_NONNULL_END
