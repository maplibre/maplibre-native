#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>

#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/** A position in a coordinate reference system, in the system's own units. */
typedef struct __attribute__((objc_boxable)) MLNCRSPosition {
  /** The x coordinate, such as an easting. */
  double x;
  /** The y coordinate, such as a northing. It grows toward the top of the map. */
  double y;
} MLNCRSPosition;

/**
 The square, power-of-two quad tile grid over a coordinate reference system that the map's tiles
 were cut in.
 */
typedef struct __attribute__((objc_boxable)) MLNTileMatrixSet {
  /** The top-left corner of tile 0/0/0: its smallest x and largest y. */
  MLNCRSPosition origin;
  /** The side of tile 0/0/0, in the system's units. */
  double extentAtZoom0;
} MLNTileMatrixSet;

/**
 A planar coordinate reference system, such as a polar stereographic or a national grid, and the
 tile grid laid over it, so a map can draw tiles that were cut in that system without warping them
 into Web Mercator.

 Register the system with ``MLNStyle/addProjection:error:`` before loading a style that names it,
 then select it by name with the style's `projection.type`. A style that names a system nobody
 registered falls back to Web Mercator. Every tile source of such a map is taken to be cut in
 ``tileMatrixSet``, and the map counts the system's units as meters wherever it converts meters,
 for example in ``MLNMapCamera/altitude``. The map never draws world copies in such a system and
 never wraps across the antimeridian.

 The map calls ``forward`` and ``inverse`` from any thread, several at once, and they must return
 the same answer for the same input. They must not call back into the map or wait on the main
 thread. The last reference to the blocks can be released on any thread, so neither may capture
 an object whose `dealloc` must run on the main thread.

 > Note: This API is experimental and may change in a future release.
 */
MLN_EXPORT
@interface MLNCoordinateReferenceSystem : NSObject

/**
 Returns a coordinate reference system.

 @param name The name the style's `projection.type` uses, such as `EPSG:3413`. It needs a colon:
    names without one are reserved for built-in projections.
 @param tileMatrixSet The tile grid the map's tiles were cut in.
 @param forward Converts a coordinate to a position in the system. Return `NaN` coordinates where
    the coordinate has no position.
 @param inverse Converts a position in the system to a coordinate. Return
    `kCLLocationCoordinate2DInvalid` where the position has no coordinate.
 */
- (instancetype)initWithName:(NSString *)name
               tileMatrixSet:(MLNTileMatrixSet)tileMatrixSet
                     forward:(MLNCRSPosition (^)(CLLocationCoordinate2D coordinate))forward
                     inverse:(CLLocationCoordinate2D (^)(MLNCRSPosition position))inverse
    NS_DESIGNATED_INITIALIZER;

- (instancetype)init NS_UNAVAILABLE;

/** The name the style's `projection.type` uses. */
@property (nonatomic, readonly, copy) NSString *name;

/** The tile grid the map's tiles were cut in. */
@property (nonatomic, readonly) MLNTileMatrixSet tileMatrixSet;

/** Converts a coordinate to a position in the system. */
@property (nonatomic, readonly, copy) MLNCRSPosition (^forward)(CLLocationCoordinate2D coordinate);

/** Converts a position in the system to a coordinate. */
@property (nonatomic, readonly, copy) CLLocationCoordinate2D (^inverse)(MLNCRSPosition position);

@end

NS_ASSUME_NONNULL_END
