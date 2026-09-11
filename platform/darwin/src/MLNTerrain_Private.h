#import <Foundation/Foundation.h>

#import "MLNTerrain.h"

namespace mln {
namespace style {
class Terrain;
}
}  // namespace mln

@interface MLNTerrain (Private)

/**
 Initializes and returns an ``MLNTerrain`` equivalent to the given `mln::style::Terrain`.
 */
- (instancetype)initWithMBGLTerrain:(const mln::style::Terrain &)mbglTerrain;

/**
 Returns an `mln::style::Terrain` representation of the ``MLNTerrain``.
 */
- (mln::style::Terrain)mbglTerrain;

@end
