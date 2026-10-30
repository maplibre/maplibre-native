#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// Registers the C-ABI `ngon` style layer type with MapLibre. Call it before a style that contains
/// `"type": "ngon"` is loaded; a style naming an unregistered type drops the layer. Safe to call
/// more than once.
@interface NgonPlugin : NSObject

+ (BOOL)registerWithError:(NSError *_Nullable *_Nullable)error;

@end

NS_ASSUME_NONNULL_END
