#import "MLNPluginStyleLayer.h"
#import "MLNPluginStyleLayer_Private.h"
#import "MLNStyleLayerManager.h"
#include <mbgl/plugin/plugin_layer.hpp>
#include <mbgl/plugin/plugin_layer_impl.hpp>
#include <mbgl/layermanager/layer_manager.hpp>
#include <mbgl/style/conversion/layer.hpp>

#import "MLNPluginLayer.h"

@implementation MLNPluginStyleLayer

- (instancetype)initWithType:(NSString *)layerType
             layerIdentifier:(NSString *)identifier
              layerPropeties:(NSDictionary *)layerPropeties {
      
    // Setup the same property paradigm that would be coming in the style
    // This at a minimum creates a dictionary that has id and type
    
    NSMutableDictionary *layerPropertiesAggregated = [NSMutableDictionary dictionary];
    if (layerPropeties) {
        [layerPropertiesAggregated addEntriesFromDictionary:layerPropeties];
    }
    [layerPropertiesAggregated setObject:identifier forKey:@"id"];
    [layerPropertiesAggregated setObject:layerType forKey:@"type"];

    mbgl::style::conversion::Error e;
    auto propertiesValue = mbgl::style::makeConvertible(layerPropertiesAggregated);
    
    // Create the layer using the same convert method that's used by the style loading
    std::optional<std::unique_ptr<mbgl::style::Layer>> converted = mbgl::style::conversion::convert<std::unique_ptr<mbgl::style::Layer>>(propertiesValue, e);
    if (!converted) {
        return nil;
    }
    auto layer = std::move(*converted);
    
    if (layer == nullptr) {
        return nil;
    }
    
    self = [super initWithPendingLayer:std::move(layer)];

    return self;
    
}



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

- (void)setPluginProperty:(NSString *)propertyName
                    value:(id)propertyValue {
    
    MLNPluginLayer *layer = [self pluginLayer];
    NSDictionary *updatedProperties = @{propertyName: propertyValue};
    [layer onUpdateLayerProperties:updatedProperties];
    
}



@end


MLNStyleLayer* mbgl::PluginLayerPeerFactory::createPeer(style::Layer *rawLayer) {
    return [[MLNPluginStyleLayer alloc] initWithRawLayer:rawLayer];

}
