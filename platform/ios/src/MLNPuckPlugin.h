#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// Registers the C-ABI `gltf` style layer type with MapLibre. Call it before a style that contains
/// `"type": "gltf"` is loaded; a style naming an unregistered type drops the layer. Safe to call more
/// than once.
///
/// Only available in builds made with `--//:plugins=true`.
@interface MLNPuckPlugin : NSObject

/// - returns: `YES` when the layer type is registered, otherwise `NO` and `error` is set. Also `NO` in a
///   build without plugin support.
+ (BOOL)registerWithError:(NSError *_Nullable *_Nullable)error;

@end

NS_ASSUME_NONNULL_END
