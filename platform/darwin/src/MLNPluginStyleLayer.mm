//
//  MLNPLuginStyleLayer.m
//  App
//
//  Created by Malcolm Toon on 5/17/25.
//

#import "MLNPluginStyleLayer.h"
#import "MLNPluginStyleLayer_Private.h"
#import <mbgl/plugins/plugin_layer.hpp>
#import <mbgl/plugins/plugin_layer_impl.hpp>
#import "MLNPluginLayer.h"

@implementation MLNPluginStyleLayer

-(void)getStats {
    
    mbgl::style::PluginLayer *l = (mbgl::style::PluginLayer *)self.rawLayer;
    auto pl = l->impl();
    
}

-(MLNPluginLayer *)pluginLayer {
    
    mbgl::style::PluginLayer *l = (mbgl::style::PluginLayer *)self.rawLayer;
    if (l->_platformReference) {
        MLNPluginLayer *pl = (__bridge MLNPluginLayer *)l->_platformReference;
        return pl;
    }
    
    return nil;
    
}


@end


MLNStyleLayer* mbgl::PluginLayerPeerFactory::createPeer(style::Layer *rawLayer) {
    return [[MLNPluginStyleLayer alloc] initWithRawLayer:rawLayer];

}
