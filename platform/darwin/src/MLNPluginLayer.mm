//
//  MLNPluginLayer.m
//  App
//
//  Created by Malcolm Toon on 4/23/25.
//

#import "MLNPluginLayer.h"

@implementation MLNPluginLayer

// This is the layer type in the style that is used
-(NSString *)layerTypeID {
    // Base class returns the class name just to return something
    return @"MLNPluginLayer";
}

// This is called from the core to create the layer with the properties
// in the style
-(void)createLayerFromProperties:(NSDictionary *)properties {
    // Base class does nothing
}


@end
