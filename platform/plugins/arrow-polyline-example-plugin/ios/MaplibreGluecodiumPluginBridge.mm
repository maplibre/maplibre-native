#import "MLNXPlatformPluginBridge.h"
#import "MaplibreGluecodiumPluginBridge.h"
#include <mbgl/plugin/cross_platform_plugin.hpp>

@implementation MaplibreGluecodiumPluginBridge {
  uint64_t _pluginPtr;
}

- (instancetype)initWithPluginPtr:(uint64_t)pluginPtr {
  self = [super init];
  if (self) {
    _pluginPtr = pluginPtr;
  }
  return self;
}

- (mbgl::platform::XPlatformPlugin&)plugin {
  return *reinterpret_cast<mbgl::platform::XPlatformPlugin*>(_pluginPtr);
}

@end
