#import "MLNPluginStyleLayer.h"
#import <mln/plugin/plugin_layer.hpp>
#import <mln/plugin/plugin_layer_impl.hpp>
#import "MLNPluginLayer.h"
#import "MLNPluginStyleLayer_Private.h"

@implementation MLNPluginStyleLayer

- (void)getStats {
  mln::style::PluginLayer *l = (mln::style::PluginLayer *)self.rawLayer;
  auto pl = l->impl();
}

- (MLNPluginLayer *)pluginLayer {
  mln::style::PluginLayer *l = (mln::style::PluginLayer *)self.rawLayer;
  if (l->_platformReference) {
    MLNPluginLayer *pl = (__bridge MLNPluginLayer *)l->_platformReference;
    return pl;
  }

  return nil;
}

@end

MLNStyleLayer *mln::PluginLayerPeerFactory::createPeer(style::Layer *rawLayer) {
  return [[MLNPluginStyleLayer alloc] initWithRawLayer:rawLayer];
}
