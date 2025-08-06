#import "PluginProtocolExample.h"

@implementation PluginProtocolExample

-(BOOL)canRequestResource:(MLNPluginProtocolHandlerResource *)resource {
    if ([resource.resourceURL containsString:@"pluginProtocol"]) {
        return YES;
    }
    return NO;
}

-(MLNPluginProtocolHandlerResponse *)requestResource:(MLNPluginProtocolHandlerResource *)resource {

    NSData *data = [NSData dataWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"PluginLayerTestStyle.json" ofType:nil]];

    MLNPluginProtocolHandlerResponse *response = [[MLNPluginProtocolHandlerResponse alloc] init];
    response.data = data;
    return response;

}


@end
