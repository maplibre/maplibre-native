#import <Foundation/Foundation.h>
#import <stdint.h>

@protocol MLNXPlatformPluginBridge;

NS_ASSUME_NONNULL_BEGIN

/// A bridge class that wraps a raw XPlatformPlugin pointer.
/// This is designed for use with Gluecodium-generated Swift bindings.
@interface MaplibreGluecodiumPluginBridge : NSObject <MLNXPlatformPluginBridge>

/// Initialize with a raw XPlatformPlugin pointer
/// @param pluginPtr The UInt64 pointer from MaplibrePlugin.ptr
- (instancetype)initWithPluginPtr:(uint64_t)pluginPtr;

@end

NS_ASSUME_NONNULL_END
