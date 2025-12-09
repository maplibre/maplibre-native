#import "MLNBasicPluginExample.h"
#import "BasicPluginExample.hpp"
#include <memory>

@implementation MLNBasicPluginExample {
  std::unique_ptr<mbgl::platform::BasicPluginExample> _plugin;
}

- (instancetype)init {
  if (self = [super init]) {
    _plugin = std::make_unique<mbgl::platform::BasicPluginExample>();
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
