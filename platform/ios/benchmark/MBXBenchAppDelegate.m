@import Mapbox;

#import "MBXBenchAppDelegate.h"
#import "MBXBenchViewController.h"

@implementation MBXBenchAppDelegate

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    
    [MGLSettings useWellKnownTileServer:MGLMapTiler];
    
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.rootViewController = [MBXBenchViewController new];
    [self.window makeKeyAndVisible];

    return YES;
}

@end
