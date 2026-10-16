#import <Foundation/Foundation.h>

#import "MLNActionJournalOptions.h"
#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/**
 The ``MLNMapOptions`` object provides a way to set map properties for each instance
 */
MLN_EXPORT
@interface MLNMapOptions : NSObject

/**
 URL of the map style to display. The URL may be a full HTTP
 or HTTPS URL, a canonical URL or a path to a local file relative
 to the application’s resource path. Specify `nil` for the default style.
 */
@property (nonatomic, nullable) NSURL *styleURL;

/**
 JSON string of the map style to display. The JSON must conform to the
 <a href="https://maplibre.org/maplibre-style-spec/">MapLibre Style Specification</a>.
 Specify `nil` for the default style.
 Ignored if `styleURL` is set to a non-nil value.
 */

@property (nonatomic, nullable) NSString *styleJSON;

/**
 Action journal  options
 */
@property (nonatomic, nonnull) MLNActionJournalOptions *actionJournalOptions;

/**
 List of plugin classes
 */
@property NSArray *pluginLayers;

/**
    Enable FastPFOR decoding for vector tiles. Defaults to `NO`.
 */
@property (nonatomic) bool fastPFOREnabled;

/**
    Enable reporting of rendered feature IDs and bounds through frame statistics
 */
@property (nonatomic) bool featureInfoEnabled;

/**
 The number of device pixels the map renders per point. Defaults to `0`, which follows the
 scale of the screen the map view is shown on.

 Set it to render the map at a resolution independent of the screen, for example when the map
 view is used offscreen to capture images or video frames of a fixed pixel size. Labels, line
 widths and icons keep their size in points, so the result looks like the same map on a screen
 of that scale. The ratio is fixed for the lifetime of the map view.

 Only applies to ``MLNMapView`` on iOS.
 */
@property (nonatomic) CGFloat pixelRatio;

@end

NS_ASSUME_NONNULL_END
