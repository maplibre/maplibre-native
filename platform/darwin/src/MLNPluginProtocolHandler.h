#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef enum {
  MLNPluginProtocolHandlerResourceKindUnknown,
  MLNPluginProtocolHandlerResourceKindStyle,
  MLNPluginProtocolHandlerResourceKindSource,
  MLNPluginProtocolHandlerResourceKindTile,
  MLNPluginProtocolHandlerResourceKindGlyphs,
  MLNPluginProtocolHandlerResourceKindSpriteImage,
  MLNPluginProtocolHandlerResourceKindSpriteJSON,
  MLNPluginProtocolHandlerResourceKindImage
} MLNPluginProtocolHandlerResourceKind;

typedef enum {
  MLNPluginProtocolHandlerResourceLoadingMethodUnknown,
  MLNPluginProtocolHandlerResourceLoadingMethodCacheOnly,
  MLNPluginProtocolHandlerResourceLoadingMethodNetworkOnly,
  MLNPluginProtocolHandlerResourceLoadingMethodAll
} MLNPluginProtocolHandlerResourceLoadingMethod;

// TODO: Might make sense to add this to it's own file
@interface MLNTileData : NSObject

// Optional Tile Data
@property NSString *tileURLTemplate;
@property int tilePixelRatio;
@property int tileX;
@property int tileY;
@property int tileZoom;

@end

@interface MLNPluginProtocolHandlerResource : NSObject

@property MLNPluginProtocolHandlerResourceKind resourceKind;

@property MLNPluginProtocolHandlerResourceLoadingMethod loadingMethod;

@property NSString *resourceURL;

// This is optional
@property MLNTileData *__nullable tileData;

@end

@interface MLNPluginProtocolHandlerResponse : NSObject

@property NSData *data;

@end

@interface MLNPluginProtocolHandler : NSObject

- (BOOL)canRequestResource:(MLNPluginProtocolHandlerResource *)resource;

- (MLNPluginProtocolHandlerResponse *)requestResource:(MLNPluginProtocolHandlerResource *)resource;

@end

NS_ASSUME_NONNULL_END
