#import <Foundation/Foundation.h>

#import "MGLFoundation.h"
#import "MGLTileServerOptions.h"

NS_ASSUME_NONNULL_BEGIN

/**
 Well-known tile servers
 */
typedef NS_ENUM(NSUInteger, MGLWellKnownTileServer) {
    /**
     Maptiler
     */
    MGLMapTiler,
    /**
     MapLibre
     */
    MGLMapLibre,
    /**
     Mapbox
     */
    MGLMapbox
};

/**
 The `MGLSettings` object provides a global way to set SDK properties such
 as apiKey, backend URL, etc.
 */
MGL_EXPORT
@interface MGLSettings : NSObject

#pragma mark Tile Server Configuration

/**
 Tile server options
 */
@property (class, copy, nullable) MGLTileServerOptions* tileServerOptions;

#pragma mark Authorizing Access

/**
 The API Key used by all instances of `MGLMapView` in the current application.
 Setting this property to a value of `nil` has no effect.

 @note You must set the API key before attempting to load any style which
    requires the token. Therefore, you should generally set it before creating an instance of
    `MGLMapView`. The recommended way to set an api key is to add an entry
    to your application’s Info.plist file with the key `MGLApiKey`
    and the type `String`. Alternatively, you may call this method from your
    application delegate’s `-applicationDidFinishLaunching:` method.
 */
@property (class, copy, nullable) NSString *apiKey;

/**
 Instructs the SDk to use the give tile server
 */
+ (void)useWellKnownTileServer:(MGLWellKnownTileServer)tileServer;

@end

NS_ASSUME_NONNULL_END
