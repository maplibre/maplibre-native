#import "MLNFoundation.h"

#import "MLNRasterTileSource.h"

NS_ASSUME_NONNULL_BEGIN

/**
 An `NSNumber` object containing an unsigned integer that specifies the encoding
 formula for raster-dem tilesets. The integer corresponds to one of
 the constants described in ``MLNDEMEncoding``.

 The default value for this option is ``MLNDEMEncoding/MLNDEMEncodingMapbox``.

 This option cannot be represented in a TileJSON or style JSON file. It is used
 with the ``MLNRasterDEMSource`` class and is ignored when creating a
 ``MLNRasterTileSource`` or ``MLNRasterTileSource`` object.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNTileSourceOption MLNTileSourceOptionDEMEncoding;

/**
 The encoding formula used to generate the raster tileset
*/

typedef NS_ENUM(NSUInteger, MLNDEMEncoding) {

  /**
     Raster tiles generated with the [Mapbox encoding
     formula](https://docs.mapbox.com/help/troubleshooting/access-elevation-data/#mapbox-terrain-rgb)
  */
  MLNDEMEncodingMapbox = 0,

  /**
   Raster tiles generated with the [Mapzen Terrarium encoding
   formula](https://aws.amazon.com/public-datasets/terrain/).
  */
  MLNDEMEncodingTerrarium = 1,
};

/**
 ``MLNRasterDEMSource`` is a map content source that supplies rasterized
 <a href="https://en.wikipedia.org/wiki/Digital_elevation_model">digital elevation model</a>
 (DEM) tiles to be shown on the map. The location of and metadata about the
 tiles are defined either by an option dictionary or by an external file that
 conforms to the
 <a href="https://github.com/mapbox/tilejson-spec/">TileJSON specification</a>.
 A raster DEM source is added to an ``MLNStyle`` object along with one or more
 ``MLNHillshadeStyleLayer`` or ``MLNColorReliefStyleLayer`` objects. Use a hillshade
 style layer to control the appearance of content supplied by the raster DEM source,
 or a color relief style layer to render elevation data with a color ramp.

 Each
 <a href="https://maplibre.org/maplibre-style-spec/#sources-raster-dem"><code>raster-dem</code></a>
 source defined by the style JSON file is represented at runtime by an
 ``MLNRasterDEMSource`` object that you can use to initialize new style layers.
 You can also add and remove sources dynamically using methods such as
 ``MLNStyle/addSource:`` and ``MLNStyle/sourceWithIdentifier:``.

 Raster DEM sources decode their tiles with the
 <a
 href="https://docs.mapbox.com/help/troubleshooting/access-elevation-data/#mapbox-terrain-rgb">Mapbox
 Terrain-RGB</a> formula unless told otherwise. For tiles in another encoding, such as Terrarium,
 use ``-initWithIdentifier:configurationURL:tileSize:options:`` with
 ``MLNTileSourceOptionDEMEncoding``, or the same option with
 ``-initWithIdentifier:tileURLTemplates:options:``.

 ### Example

 ```swift
 let terrainRGBURL = URL(string: "maptiler://sources/terrain-rgb")!
 let source = MLNRasterDEMSource(identifier: "hills", configurationURL: terrainRGBURL)
 mapView.style?.addSource(source)
 ```
 */
MLN_EXPORT
@interface MLNRasterDEMSource : MLNRasterTileSource

/**
 Returns a raster DEM source initialized with an identifier, configuration URL,
 tile size, and options.

 After initializing and configuring the source, add it to a map view’s style
 using the ``MLNStyle/addSource:`` method.

 The URL may be a full HTTP or HTTPS URL or canonical URL, for example a
 `pmtiles://` URL. The URL should point to a JSON file that conforms to the
 <a href="https://github.com/mapbox/tilejson-spec/">TileJSON specification</a>,
 or to a PMTiles archive. Neither can carry the elevation encoding, so pass
 ``MLNTileSourceOptionDEMEncoding`` in `options` for tiles that are not
 Mapbox Terrain-RGB.

 @param identifier A string that uniquely identifies the source in the style to
    which it is added.
 @param configurationURL A URL to a TileJSON configuration file or PMTiles
    archive describing the source’s contents and other metadata.
 @param tileSize The width and height (measured in points) of each tiled image
    in the raster DEM source. See the ``MLNTileSourceOptionTileSize``
    documentation for details.
 @param options A dictionary containing configuration options. Only
    ``MLNTileSourceOptionDEMEncoding`` is used. Pass in `nil` to use the
    default values.
 @return An initialized raster DEM source.
 */
- (instancetype)initWithIdentifier:(NSString *)identifier
                  configurationURL:(NSURL *)configurationURL
                          tileSize:(CGFloat)tileSize
                           options:(nullable NSDictionary<MLNTileSourceOption, id> *)options;

/**
 The encoding formula the source decodes its tiles with.

 This is the value of ``MLNTileSourceOptionDEMEncoding`` the source was
 initialized with, or ``MLNDEMEncoding/MLNDEMEncodingMapbox`` if it was given
 none.
 */
@property (nonatomic, readonly) MLNDEMEncoding encoding;

@end

NS_ASSUME_NONNULL_END
