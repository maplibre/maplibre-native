#import "MLNPluginProtocolHandler.h"

@implementation MLNPluginProtocolHandler

-(BOOL)canRequestResource:(MLNPluginProtocolHandlerResource *)resource {
    // Base class returns false
    return NO;
}

-(MLNPluginProtocolHandlerResponse *)requestResource:(MLNPluginProtocolHandlerResource *)resource {
    // Base class does nothing
    return nil;
}


@end

@implementation MLNPluginProtocolHandlerResource
@end


@implementation MLNPluginProtocolHandlerResponse
@end

@implementation MLNTileData
@end
