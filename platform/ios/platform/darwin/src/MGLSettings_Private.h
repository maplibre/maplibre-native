#import "MGLSettings.h"
#import <mbgl/util/tile_server_options.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface MGLSettings (Private)

/// Returns the shared instance of the `MGLSettings` class.
@property (class, nonatomic, readonly) MGLSettings *sharedSettings;

/// The current global access token.
@property (atomic, nullable) NSString *apiKey;

/// The API base URL that is used to access Mapbox resources. The default base URL is `https://api.mapbox.com`. If `nil`, the Mapbox default base API URL is in use.
@property (atomic, readwrite, nullable) NSURL *apiBaseURL;

@property (atomic, readwrite, nullable) mbgl::TileServerOptions *tileServerOptions;

/// Used for observing tiile server configuration changes
@property (atomic, nullable) NSString *tileServerOptionsChangeToken;

/// called internally to signal tile server options changed
+ (void)tileServerOptionsChanged;

@end

NS_ASSUME_NONNULL_END
