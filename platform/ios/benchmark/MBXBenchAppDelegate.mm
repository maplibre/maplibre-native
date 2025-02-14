#import "Mapbox.h"

#import "MBXBenchAppDelegate.h"
#import "MBXBenchViewController.h"

@implementation MBXBenchAppDelegate

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {

#ifndef MLN_LOGGING_DISABLED
    [MLNLoggingConfiguration sharedConfiguration].loggingLevel = MLNLoggingLevelFault;
#endif

    [MLNSettings useWellKnownTileServer:MLNMapTiler];

    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.rootViewController = [MBXBenchViewController new];
    [self.window makeKeyAndVisible];

    return YES;
}

@end
