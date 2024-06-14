#import "MLNFoundation.h"

#import "MLNRasterTileSource.h"

/**
 An `NSNumber` object containing an unsigned integer that specifies the encoding
 formula for raster-dem tilesets. The integer corresponds to one of
 the constants described in `MLNDEMEncoding`.

 The default value for this option is `MLNDEMEncodingMapbox`.

 This option cannot be represented in a TileJSON or style JSON file. It is used
 with the `MLNRasterDEMSource` class and is ignored when creating an
 `MLNRasterTileSource` or `MLNVectorTileSource` object.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNTileSourceOption MLNTileSourceOptionDEMEncoding;

/**
 `MLNRasterDEMSource` is a map content source that supplies rasterized
 <a href="https://en.wikipedia.org/wiki/Digital_elevation_model">digital elevation model</a>
 (DEM) tiles to be shown on the map. The location of and metadata about the
 tiles are defined either by an option dictionary or by an external file that
 conforms to the
 <a href="https://github.com/mapbox/tilejson-spec/">TileJSON specification</a>.
 A raster DEM source is added to an `MLNStyle` object along with one or more
 `MLNHillshadeStyleLayer` objects. Use a hillshade style layer to control the
 appearance of content supplied by the raster DEM source.

 Each
 <a href="https://maplibre.org/maplibre-style-spec/#sources-raster-dem"><code>raster-dem</code></a>
 source defined by the style JSON file is represented at runtime by an
 `MLNRasterDEMSource` object that you can use to initialize new style layers.
 You can also add and remove sources dynamically using methods such as
 `-[MLNStyle addSource:]` and `-[MLNStyle sourceWithIdentifier:]`.

 Currently, raster DEM sources only support the format used by
 <a
 href="https://docs.mapbox.com/help/troubleshooting/access-elevation-data/#mapbox-terrain-rgb">Mapbox
 Terrain-RGB</a>.

 ### Example

 ```swift
 let terrainRGBURL = URL(string: "maptiler://sources/terrain-rgb")!
 let source = MLNRasterDEMSource(identifier: "hills", configurationURL: terrainRGBURL)
 mapView.style?.addSource(source)
 ```
 */
MLN_EXPORT
@interface MLNRasterDEMSource : MLNRasterTileSource

@end
