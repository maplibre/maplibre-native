#import "PluginLayerExample.h"

@implementation PluginLayerExample


// This is the layer type in the style that is used
+(MLNPluginLayerCapabilities *)layerCapabilities {

    MLNPluginLayerCapabilities *tempResult = [[MLNPluginLayerCapabilities alloc] init];
    tempResult.layerID = @"maplibre::filter_features";
    tempResult.requiresPass3D = YES;
    tempResult.supportsReadingTileFeatures = YES;
    return tempResult;

}

// The overrides
-(void)onRenderLayer:(MLNMapView *)mapView
       renderEncoder:(id<MTLRenderCommandEncoder>)renderEncoder {
   //NSLog(@"PluginLayerExample: On Render Layer");

}

-(void)onUpdateLayer {

}

//-(void)onBucketLoaded:(MLNRawBucket *)bucket {
//
//}

-(void)onUpdateLayerProperties:(NSDictionary *)layerProperties {
  //  NSLog(@"Layer Properties: %@", layerProperties);
}

-(void)featureLoaded:(MLNPluginLayerTileFeature *)tileFeature {
    NSLog(@"Tile Feature (id:%@) Properties: %@", tileFeature.featureID, tileFeature.featureProperties);

    for (NSValue *v in tileFeature.featureCoordinates) {


//        NSValue *value = [NSValue valueWithBytes:&c objCType:@encode(CLLocationCoordinate2D)];
//        [featureCoordinates addObject:value];

        CLLocationCoordinate2D coord;
        [v getValue:&coord];

    }
}

-(void)featureUnloaded:(MLNPluginLayerTileFeature *)tileFeature {
   // NSLog(@"Tile Features Unloaded: %@", tileFeature.featureProperties);

}


- (void)onFeatureLoaded:(MLNPluginLayerTileFeature *)tileFeature {

    [self featureLoaded:tileFeature];

}

/// Called when a set of features are loaded from the tile
- (void)onFeatureCollectionLoaded:(MLNPluginLayerTileFeatureCollection *)tileFeatureCollection {
    //NSLog(@"Feature Collection Loaded for tile: %@", tileFeatureCollection.tileID);
    for (MLNPluginLayerTileFeature *feature in tileFeatureCollection.features) {
        [self featureLoaded:feature];
    }

}

/// Called when a set of features are unloaded because the tile goes out of scene/etc
- (void)onFeatureCollectionUnloaded:(MLNPluginLayerTileFeatureCollection *)tileFeatureCollection {
    //NSLog(@"Feature Collection Unloaded for tile: %@", tileFeatureCollection.tileID);
    for (MLNPluginLayerTileFeature *feature in tileFeatureCollection.features) {
        [self featureUnloaded:feature];
    }
}

@end
