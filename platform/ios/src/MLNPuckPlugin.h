#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// Registers the C-ABI `puck` and `gltf` style layer types with MapLibre. Call it before a
/// style that contains `"type": "puck"` is loaded; a style naming an unregistered type drops the
/// layer. Safe to call more than once.
@interface MLNPuckPlugin : NSObject

/// - returns: `YES` when both layer types are registered, otherwise `NO` and `error` is set.
+ (BOOL)registerWithError:(NSError *_Nullable *_Nullable)error;

@end

NS_ASSUME_NONNULL_END
