#import "Mapbox.h"

#import "MBXAppDelegate.h"
#import "MBXViewController.h"

#if __has_feature(undefined_behavior_sanitizer) || defined(__SANITIZE_UNDEFINED_BEHAVIOR__)
#import "sanitizer/ubsan_interface.h"
const char* __ubsan_default_options(void) {
    static char default_opts[2 * PATH_MAX];
    NSString* path = [[NSBundle bundleForClass:[MBXAppDelegate class]] pathForResource:@"metal-cpp-ignores" ofType:@"txt" inDirectory:@"Mapbox.bundle"];
    NSString* opts = @"print_stacktrace=1,halt_on_error=1,report_error_type=1,verbosity=0";  // add ',help=1' to see available options
    if (path) {
        opts = [opts stringByAppendingFormat:@",suppressions=%@", path];
    }
    NSLog(@"Using ubsan options: %@", opts);

    bzero(default_opts, sizeof(default_opts));
    strncpy(default_opts, [opts UTF8String], sizeof(default_opts) - 1);
    return default_opts;
}
#endif

@interface MBXAppDelegate()

@end

@implementation MBXAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
#ifndef MLN_LOGGING_DISABLED
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
