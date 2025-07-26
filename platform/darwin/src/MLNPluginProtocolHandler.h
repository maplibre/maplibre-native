#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MLNPluginProtocolHandlerResource : NSObject

@property NSString *resourceURL;

@end

@interface MLNPluginProtocolHandlerResponse : NSObject

@property NSData *data;

@end

@interface MLNPluginProtocolHandler : NSObject

- (BOOL)canRequestResource:(MLNPluginProtocolHandlerResource *)resource;

- (MLNPluginProtocolHandlerResponse *)requestResource:(MLNPluginProtocolHandlerResource *)resource;

@end

NS_ASSUME_NONNULL_END
