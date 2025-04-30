//
//  PluginLayerExampleMetalRendering.m
//  MapLibre
//
//  Created by Malcolm Toon on 4/29/25.
//

#import "PluginLayerExampleMetalRendering.h"

@implementation PluginLayerExampleMetalRendering


// This is the layer type in the style that is used
-(MLNPluginLayerCapabilities *)layerCapabilities {

    MLNPluginLayerCapabilities *tempResult = [[MLNPluginLayerCapabilities alloc] init];
    tempResult.layerID = @"plugin-layer-metal-rendering";
    tempResult.tileKind = MLNPluginLayerTileKindNotRequired;
    tempResult.requiresPass3D = YES;
    return tempResult;

}

// This is called from the core to create the layer with the properties
// in the style
-(void)createLayerFromProperties:(NSDictionary *)properties {

}

// The overrides
-(void)onRenderLayer {
    NSLog(@"On Render Layer");
}

-(void)onUpdateLayer {

}

-(void)onUpdateLayerProperties:(NSDictionary *)layerProperties {
    NSLog(@"Metal Layer Rendering Properties: %@", layerProperties);
}

@end
