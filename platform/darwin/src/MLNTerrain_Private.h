#import "MLNTerrain.h"
#import "MLNTypes.h"

#include <mln/map/mode.hpp>
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

// The public enums mirror the core enums value for value.
static_assert(static_cast<uint8_t>(mln::TerrainLoadMode::Quality) == MLNTerrainLoadModeQuality &&
                  static_cast<uint8_t>(mln::TerrainLoadMode::Balanced) ==
                      MLNTerrainLoadModeBalanced &&
                  static_cast<uint8_t>(mln::TerrainLoadMode::Performance) ==
                      MLNTerrainLoadModePerformance,
              "MLNTerrainLoadMode must match mln::TerrainLoadMode");
static_assert(static_cast<uint8_t>(mln::TerrainSkirtLength::Auto) == MLNTerrainSkirtLengthAuto &&
                  static_cast<uint8_t>(mln::TerrainSkirtLength::None) == MLNTerrainSkirtLengthNone,
              "MLNTerrainSkirtLength must match mln::TerrainSkirtLength");

NS_INLINE mln::TerrainLoadMode MLNTerrainLoadModeToMBGL(MLNTerrainLoadMode mode) {
  return static_cast<mln::TerrainLoadMode>(mode);
}

NS_INLINE MLNTerrainLoadMode MLNTerrainLoadModeFromMBGL(mln::TerrainLoadMode mode) {
  return static_cast<MLNTerrainLoadMode>(mode);
}

NS_INLINE mln::TerrainSkirtLength MLNTerrainSkirtLengthToMBGL(MLNTerrainSkirtLength length) {
  return static_cast<mln::TerrainSkirtLength>(length);
}

NS_INLINE MLNTerrainSkirtLength MLNTerrainSkirtLengthFromMBGL(mln::TerrainSkirtLength length) {
  return static_cast<MLNTerrainSkirtLength>(length);
}

NS_ASSUME_NONNULL_END
