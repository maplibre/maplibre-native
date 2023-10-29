#import "Mapbox.h"

#import "MBXAppDelegate.h"
#import "MBXViewController.h"

@interface MBXAppDelegate()

@end

@implementation MBXAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
#ifndef MLN_DISABLE_LOGGING
    [MLNLoggingConfiguration sharedConfiguration].loggingLevel = MLNLoggingLevelFault;
#endif
    [MLNSettings useWellKnownTileServer:MLNMapTiler];

    return YES;
}

// MARK: - Quick actions

- (void)application:(UIApplication *)application performActionForShortcutItem:(UIApplicationShortcutItem *)shortcutItem completionHandler:(void (^)(BOOL))completionHandler {
    completionHandler([self handleShortcut:shortcutItem]);
}

- (BOOL)handleShortcut:(UIApplicationShortcutItem *)shortcut {
    if ([[shortcut.type componentsSeparatedByString:@"."].lastObject isEqual:@"settings"]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [[UIApplication sharedApplication] openURL:[NSURL URLWithString:UIApplicationOpenSettingsURLString] options:@{} completionHandler:^(BOOL success) {
                if (success) {
                     NSLog(@"Opened url");
                }
            }];
        });

        return YES;
    }

    return NO;
}

@end
