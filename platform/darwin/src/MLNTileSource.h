#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNSource.h"
#import "MLNTypes.h"

NS_ASSUME_NONNULL_BEGIN

@class MLNAttributionInfo;

/**
 Options for `MLNTileSource` objects.
 */
typedef NSString *MLNTileSourceOption NS_STRING_ENUM;

/**
 An `NSNumber` object containing an unsigned integer that specifies the minimum
 zoom level at which to display tiles from the source.

 The value should be between 0 and 22, inclusive, and less than
 `MLNTileSourceOptionMaximumZoomLevel`, if specified. The default value for this
 option is 0.

 This option corresponds to the `minzoom` key in the
 <a href="https://github.com/mapbox/tilejson-spec/tree/master/2.1.0">TileJSON</a>
 specification.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNTileSourceOption MLNTileSourceOptionMinimumZoomLevel;

/**
 An `NSNumber` object containing an unsigned integer that specifies the maximum
 zoom level at which to display tiles from the source.

 The value should be between 0 and 22, inclusive, and less than
 `MLNTileSourceOptionMinimumZoomLevel`, if specified. The default value for this
 option is 22.

 This option corresponds to the `maxzoom` key in the
 <a href="https://github.com/mapbox/tilejson-spec/tree/master/2.1.0">TileJSON</a>
 specification.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNTileSourceOption MLNTileSourceOptionMaximumZoomLevel;

/**
 An `NSValue` object containing an `MLNCoordinateBounds` struct that specifies
 the geographic extent of the source.

 If this option is specified, the SDK avoids requesting any tile that falls
 outside of the coordinate bounds. Otherwise, the SDK requests any tile needed
 to cover the viewport, as it does by default.

 This option corresponds to the `bounds` key in the
 <a href="https://github.com/mapbox/tilejson-spec/tree/master/2.1.0">TileJSON</a>
 specification.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNTileSourceOption MLNTileSourceOptionCoordinateBounds;

#if TARGET_OS_IPHONE
/**
 An HTML string defining the buttons to be displayed in an action sheet when the
 source is part of a map view’s style and the map view’s attribution button is
 pressed.

 By default, no attribution statements are displayed. If the
 `MLNTileSourceOptionAttributionInfos` option is specified, this option is
 ignored.

 This option corresponds to the `attribution` key in the
 <a href="https://github.com/mapbox/tilejson-spec/tree/master/2.1.0">TileJSON</a>
 specification.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNTileSourceOption MLNTileSourceOptionAttributionHTMLString;

/**
 An array of `MLNAttributionInfo` objects defining the buttons to be displayed
 in an action sheet when the source is part of a map view’s style and the map
 view’s attribution button is pressed.

 By default, no attribution statements are displayed.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNTileSourceOption MLNTileSourceOptionAttributionInfos;
#else
/**
 An HTML string defining the buttons to be displayed in the map view’s
 attribution view when the source is part of the map view’s style.

 By default, no attribution statements are displayed. If the
 `MLNTileSourceOptionAttributionInfos` option is specified, this option is
 ignored.

 This option corresponds to the `attribution` key in the
 <a href="https://github.com/mapbox/tilejson-spec/tree/master/2.1.0">TileJSON</a>
 specification.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNTileSourceOption MLNTileSourceOptionAttributionHTMLString;

/**
 An array of `MLNAttributionInfo` objects defining the buttons to be displayed
 in the map view’s attribution view when the source is part of the map view’s
 style.

 By default, no attribution statements are displayed.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNTileSourceOption MLNTileSourceOptionAttributionInfos;
#endif

/**
 An `NSNumber` object containing an unsigned integer that specifies the tile
 coordinate system for the source’s tile URLs. The integer corresponds to one of
 the constants described in `MLNTileCoordinateSystem`.

 The default value for this option is `MLNTileCoordinateSystemXYZ`.

 This option corresponds to the `scheme` key in the
 <a href="https://github.com/mapbox/tilejson-spec/tree/master/2.1.0">TileJSON</a>
 specification.
 */
FOUNDATION_EXTERN MLN_EXPORT const MLNTileSourceOption MLNTileSourceOptionTileCoordinateSystem;

/**
 Tile coordinate systems that determine how tile coordinates in tile URLs are
 interpreted.
 */
typedef NS_ENUM(NSUInteger, MLNTileCoordinateSystem) {
  /**
   The origin is at the top-left (northwest), and `y` values increase
   southwards.

   This tile coordinate system is used by Mapbox and OpenStreetMap tile
   servers.
   */
  MLNTileCoordinateSystemXYZ = 0,

  /**
   The origin is at the bottom-left (southwest), and `y` values increase
   northwards.

   This tile coordinate system is used by tile servers that conform to the
   <a href="http://wiki.osgeo.org/wiki/Tile_Map_Service_Specification">Tile Map Service
   Specification</a>.
   */
  MLNTileCoordinateSystemTMS
};

/**
 The encoding formula used to generate the raster-dem tileset
*/

typedef NS_ENUM(NSUInteger, MLNDEMEncoding) {

  /**
   Raster tiles generated with the [Mapbox encoding
   formula](https://docs.mapbox.com/help/troubleshooting/access-elevation-data/#mapbox-terrain-rgb).
  */
  MLNDEMEncodingMapbox = 0,

  /**
   Raster tiles generated with the [Mapzen Terrarium encoding
   formula](https://aws.amazon.com/public-datasets/terrain/).
  */
  MLNDEMEncodingTerrarium
};

/**
 `MLNTileSource` is a map content source that supplies map tiles to be shown on
 the map. The location of and metadata about the tiles are defined either by an
 option dictionary or by an external file that conforms to the
 <a href="https://github.com/mapbox/tilejson-spec/">TileJSON specification</a>.
 A tile source is added to an `MLNStyle` object along with one or more
 `MLNRasterStyleLayer` or `MLNVectorStyleLayer` objects. Use a style layer to
 control the appearance of content supplied by the tile source.

 A tile source is also known as a tile set. To learn about the structure of a
 Mapbox-hosted tile set, view it in
 <a href="https://www.mapbox.com/studio/tilesets/">Mapbox Studio’s Tilesets editor</a>.

 Create instances of `MLNRasterTileSource` and `MLNVectorTileSource` in order
 to use `MLNTileSource`'s properties and methods. Do not create instances of
 `MLNTileSource` directly, and do not create your own subclasses of this class.
 */
MLN_EXPORT
@interface MLNTileSource : MLNSource

// MARK: Accessing a Source’s Content

/**
 The URL to the TileJSON configuration file that specifies the contents of the
 source.

 If the receiver was initialized using
 `-initWithIdentifier:tileURLTemplates:options`, this property is set to `nil`.
 */
@property (nonatomic, copy, nullable, readonly) NSURL *configurationURL;

// MARK: Accessing Attribution Strings

/**
 An array of `MLNAttributionInfo` objects that define the attribution
 statements to be displayed when the map is shown to the user.

 By default, this array is empty. If the source is initialized with a
 configuration URL, this array is also empty until the configuration JSON file
 is loaded.
 */
@property (nonatomic, copy, readonly) NSArray<MLNAttributionInfo *> *attributionInfos;

@end

NS_ASSUME_NONNULL_END
