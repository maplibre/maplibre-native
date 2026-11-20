#import <CoreLocation/CoreLocation.h>
#import <UIKit/UIKit.h>

@interface MBXBenchViewController : UIViewController

/// Counts cache misses from existing HTTP log lines. Call before the map loads.
+ (void)installCacheMissLogger;

@end
