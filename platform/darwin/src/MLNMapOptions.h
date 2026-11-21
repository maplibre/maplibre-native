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
 How far past the vertical a line label kept upright may read before it is turned over, in
 degrees. Defaults to `0`: a label is turned over as soon as it would read from right to left,
 decided anew on every frame, as in MapLibre GL JS.

 With a margin, a label keeps the reading direction it was last drawn with until that direction is
 more than the margin past the vertical of the screen, and all copies of a label share it. This
 keeps labels on lines close to the vertical, such as the road ahead in a map rotated with the
 direction of travel, from turning over and back as the map rotates. A label drawn for the first
 time is decided as without the margin.

 It applies to text placed along lines with `text-keep-upright` and `text-rotation-alignment`
 `map`. Icons, `icon-keep-upright` included, still turn over as before, and so does the background
 of an `icon-text-fit` icon: while a line is within the margin of the vertical, a label and its
 icon can face opposite ways.

 The margin is read when the map view is created and applies to every such label in every layer;
 it cannot be changed later or limited to some layers. It ranges from 0 to 45: values outside that
 range are clamped, NaN is ignored.
 */
@property (nonatomic) float keepUprightMargin;

@end

NS_ASSUME_NONNULL_END
