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

@end

NS_ASSUME_NONNULL_END
