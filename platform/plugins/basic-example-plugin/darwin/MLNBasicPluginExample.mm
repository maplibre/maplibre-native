#import "MLNXPlatformPluginBridge.h"
#import "MLNBasicPluginExample.h"
#import "BasicPluginExample.hpp"
#include <memory>

@implementation MLNBasicPluginExample {
  std::unique_ptr<plugin::ex::BasicPluginExample> _plugin;
}

- (instancetype)init {
  if (self = [super init]) {
    _plugin = std::make_unique<plugin::ex::BasicPluginExample>();
  }
  return self;
}

- (mbgl::platform::XPlatformPlugin&)plugin {
  return *_plugin;
}

- (void)showSanFrancisco {
  _plugin->showSanFrancisco();
}

@end
