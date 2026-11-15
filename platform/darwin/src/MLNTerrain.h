#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/**
 An ``MLNTerrain`` object describes the 3D terrain of a style: the raster-dem
 source that supplies the elevation data, and a vertical exaggeration. It is the
 counterpart of the `terrain` object of the
 <a href="https://maplibre.org/maplibre-style-spec/terrain/">style specification</a>.

 To drape the map over the elevation data, set ``MLNStyle/terrain``. To make the
 map flat again, set it to `nil`. Terrain objects are immutable. To change the
 exaggeration, set a new object.

 It is recommended to give terrain its own ``MLNRasterDEMSource``, even when it
 uses the same tiles as a hillshade layer, so styles stay portable with
 MapLibre GL JS, which keeps a separate terrain source cache.
 */
MLN_EXPORT
@interface MLNTerrain : NSObject <NSCopying>

/**
 The identifier of the ``MLNRasterDEMSource`` that supplies the elevation data.
 */
@property (nonatomic, copy, readonly) NSString *sourceIdentifier;

/**
 The vertical exaggeration multiplier applied to the elevation data.

 A value of `1` renders true-scale elevation, larger values make the relief more
 pronounced, and `0` flattens it. The value is stored with the precision of the
 `float` the renderer uses, so a terrain read back from ``MLNStyle/terrain``
 equals the one that was set.
 */
@property (nonatomic, readonly) CGFloat exaggeration;

/**
 Returns a terrain object that uses the given source and exaggeration.

 @param sourceIdentifier The identifier of the ``MLNRasterDEMSource`` that
    supplies the elevation data.
 @param exaggeration The vertical exaggeration multiplier.
 */
- (instancetype)initWithSourceIdentifier:(NSString *)sourceIdentifier
                            exaggeration:(CGFloat)exaggeration NS_DESIGNATED_INITIALIZER;

/**
 Returns a terrain object that uses the given source at true scale, an
 exaggeration of `1`.

 @param sourceIdentifier The identifier of the ``MLNRasterDEMSource`` that
    supplies the elevation data.
 */
- (instancetype)initWithSourceIdentifier:(NSString *)sourceIdentifier;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
