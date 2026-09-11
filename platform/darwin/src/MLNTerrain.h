#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/**
 ``MLNTerrain`` describes a style's 3D terrain configuration.

 Terrain drapes the map over the elevation data of a raster-dem source. Per the
 <a href="https://maplibre.org/maplibre-style-spec/terrain/">style specification</a>,
 `exaggeration` is a vertical multiplier where `1.0` renders true-scale elevation.

 It is recommended to give terrain its own raster-dem source, even when it uses the
 same tiles as a hillshade source, so styles stay portable with maplibre-gl-js, which
 keeps a separate terrain source cache.

 To enable terrain, set ``MLNStyle/terrain`` to an ``MLNTerrain`` object, or use the
 ``MLNMapView/setTerrainWithSourceIdentifier:exaggeration:`` convenience method.
 */
MLN_EXPORT
@interface MLNTerrain : NSObject <NSCopying>

/**
 The identifier of the raster-dem source that supplies the elevation data.
 */
@property (nonatomic, copy, readonly) NSString *sourceIdentifier;

/**
 The vertical exaggeration multiplier applied to the elevation data.

 A value of `1.0` (the default) renders true-scale elevation. Larger values make the
 terrain more dramatic; smaller values flatten it.
 */
@property (nonatomic, readonly) CGFloat exaggeration;

/**
 Returns a terrain object with the given source identifier and exaggeration.

 @param sourceIdentifier The identifier of the raster-dem source that supplies the
    elevation data.
 @param exaggeration The vertical exaggeration multiplier to apply to the elevation
    data.
 */
- (instancetype)initWithSourceIdentifier:(NSString *)sourceIdentifier
                             exaggeration:(CGFloat)exaggeration NS_DESIGNATED_INITIALIZER;

/**
 Returns a terrain object with the given source identifier and the default
 exaggeration of `1.0`.

 @param sourceIdentifier The identifier of the raster-dem source that supplies the
    elevation data.
 */
- (instancetype)initWithSourceIdentifier:(NSString *)sourceIdentifier;

- (instancetype)init NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
