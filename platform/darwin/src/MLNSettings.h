#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNTileServerOptions.h"

NS_ASSUME_NONNULL_BEGIN

/**
 Well-known tile servers
 */
typedef NS_ENUM(NSUInteger, MLNWellKnownTileServer) {
  /**
   Maptiler
   */
  MLNMapTiler,
  /**
   MapLibre
   */
  MLNMapLibre,
  /**
   Mapbox
   */
  MLNMapbox
};

/**
 The `MLNSettings` object provides a global way to set SDK properties such
 as apiKey, backend URL, etc.
 */
MLN_EXPORT
@interface MLNSettings : NSObject

// MARK: Tile Server Configuration

/**
 Tile server options
 */
@property (class, copy, nullable) MLNTileServerOptions* tileServerOptions;

// MARK: Authorizing Access

/**
 The API Key used by all instances of ``MLNMapView`` in the current application.
 Setting this property to a value of `nil` has no effect.

 @note You must set the API key before attempting to load any style which
    requires the token. Therefore, you should generally set it before creating an instance of
    ``MLNMapView``. The recommended way to set an api key is to add an entry
    to your application’s Info.plist file with the key `MLNApiKey`
    and the type `String`. Alternatively, you may call this method from your
    application delegate’s `-applicationDidFinishLaunching:` method.
 */
@property (class, copy, nullable) NSString* apiKey;

/**
 Instructs the SDk to use the give tile server
 */
+ (void)useWellKnownTileServer:(MLNWellKnownTileServer)tileServer;

@end

NS_ASSUME_NONNULL_END
