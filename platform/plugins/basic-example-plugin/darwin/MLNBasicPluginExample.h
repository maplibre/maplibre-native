#import <Foundation/Foundation.h>
#import "MLNXPlatformPluginBridge.h"

NS_ASSUME_NONNULL_BEGIN

/// A basic example MapLibre plugin that demonstrates the cross-platform plugin architecture.
///
/// This plugin logs lifecycle events and provides a method to navigate to San Francisco.
///
/// Usage:
/// ```objc
/// MLNBasicPluginExample *plugin = [[MLNBasicPluginExample alloc] init];
/// MLNMapView *mapView = [[MLNMapView alloc] initWithFrame:frame styleURL:nil plugins:@[plugin]];
/// [plugin showSanFrancisco];
/// ```
@interface MLNBasicPluginExample : NSObject <MLNXPlatformPluginBridge>

/// Bridge a function via objc
- (void)showSanFrancisco;

@end

NS_ASSUME_NONNULL_END
