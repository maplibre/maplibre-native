#import <Foundation/Foundation.h>

#ifdef __cplusplus
#include <memory>
#endif

#ifdef __cplusplus
namespace mbgl {
namespace platform {
class XPlatformPlugin;
}
}
#endif

NS_ASSUME_NONNULL_BEGIN

@protocol MLNXPlatformPluginBridge <NSObject>

#ifdef __cplusplus
- (mbgl::platform::XPlatformPlugin&)plugin;
#endif

@end

NS_ASSUME_NONNULL_END
