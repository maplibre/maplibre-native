#import "MapLibre.h"

#import "MLNLoggingConfiguration.h"

#import "MBXBenchAppDelegate.h"
#import "MBXBenchViewController.h"

@implementation MBXBenchAppDelegate

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
#ifndef MLN_LOGGING_DISABLED
  // Enable debug logging so we can see cache miss messages
  [MLNLoggingConfiguration sharedConfiguration].loggingLevel =
#if MLN_LOGGING_ENABLE_DEBUG
      MLNLoggingLevelDebug;
#else
      MLNLoggingLevelVerbose;
#endif
  [MBXBenchViewController installCacheMissLogger];
#endif

  [MLNSettings useWellKnownTileServer:MLNMapTiler];

  self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  self.window.rootViewController = [MBXBenchViewController new];
  [self.window makeKeyAndVisible];

  return YES;
}

@end
