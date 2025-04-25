//
//  PluginLayerExample.m
//  App
//
//  Created by Malcolm Toon on 4/23/25.
//

#import "PluginLayerExample.h"

@implementation PluginLayerExample

// This is the layer type in the style that is used
-(NSString *)layerTypeID {
    return @"plugin-layer-test";
}

// This is called from the core to create the layer with the properties
// in the style
-(void)createLayerFromProperties:(NSDictionary *)properties {

}

@end
