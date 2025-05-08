//
//  MLNPluginLayer.m
//  App
//
//  Created by Malcolm Toon on 4/23/25.
//

#import "MLNPluginLayer.h"

@implementation MLNPluginLayer

// This is the layer type in the style that is used
+(MLNPluginLayerCapabilities *)layerCapabilities {

    // Base class returns the class name just to return something
    // TODO: Add an assert/etc or something to notify the developer that this needs to be overridden
    return nil;

}

- (void)onRenderLayer:(MLNMapView *)mapView
        renderEncoder:(id<MTLRenderCommandEncoder>)renderEncoder {
    // Base class does nothing
}

- (void)onUpdateLayer {
    // Base class does nothing
}

-(void)onUpdateLayerProperties:(NSDictionary *)layerProperties {
    // Base class does nothing
}

/// Added to a map view
- (void)didMoveToMapView:(MLNMapView *)mapView {
    // Base class does nothing
}


@end



@implementation MLNPluginLayerCapabilities
@end
