#import "MLNSettings.h"
#import <mbgl/util/tile_server_options.hpp>
#import <mbgl/util/default_style.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface MLNSettings (Private)

/// Returns the shared instance of the `MLNSettings` class.
@property (class, nonatomic, readonly) MLNSettings *sharedSettings;

/// The current global access token.
@property (atomic, nullable) NSString *apiKey;

/// internal access to tile server options
@property (atomic, readwrite, nullable) mbgl::TileServerOptions *tileServerOptionsInternal;

/// tile server base url
@property (atomic, readwrite, nullable) NSURL *apiBaseURL;

/// Used for observing tiile server configuration changes
@property (atomic, nullable) NSString *tileServerOptionsChangeToken;

/// called internally to signal tile server options changed
+ (void)tileServerOptionsChanged;

@end

NS_ASSUME_NONNULL_END
