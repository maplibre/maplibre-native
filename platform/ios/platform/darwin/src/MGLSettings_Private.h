#import "MGLSettings.h"

NS_ASSUME_NONNULL_BEGIN

/// NSUserDefaults key that controls developer account type
FOUNDATION_EXTERN NSString * const MGLMapboxAccountTypeKey;

@interface MGLSettings (Private)

/// Returns the shared instance of the `MGLSettings` class.
@property (class, nonatomic, readonly) MGLSettings *sharedSettings;

/// The current global access token.
@property (atomic, nullable) NSString *accessToken;

/// The API base URL that is used to access Mapbox resources. The default base URL is `https://api.mapbox.com`. If `nil`, the Mapbox default base API URL is in use.
@property (atomic, readwrite, nullable) NSURL *apiBaseURL;

@end

NS_ASSUME_NONNULL_END
