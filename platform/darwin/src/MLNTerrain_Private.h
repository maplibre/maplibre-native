#import "MLNTerrain.h"

#include <mln/style/terrain.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface MLNTerrain (Private)

/**
 Initializes and returns an ``MLNTerrain`` equivalent to the given
 `mln::style::Terrain`.
 */
- (instancetype)initWithMBGLTerrain:(const mln::style::Terrain &)mbglTerrain;

/**
 Returns an `mln::style::Terrain` equivalent to the ``MLNTerrain``.
 */
- (mln::style::Terrain)mbglTerrain;

@end

NS_ASSUME_NONNULL_END
