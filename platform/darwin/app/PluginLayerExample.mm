#import "PluginLayerExample.h"

@interface PluginLayerExample () {

}

@property BOOL logFeatures;

@end


@implementation PluginLayerExample


// This is the layer type in the style that is used
+(MLNPluginLayerCapabilities *)layerCapabilities {

    MLNPluginLayerCapabilities *tempResult = [[MLNPluginLayerCapabilities alloc] init];
    tempResult.layerID = @"maplibre::filter_features";
    tempResult.requiresPass3D = YES;
    tempResult.supportsReadingTileFeatures = YES;
    return tempResult;

}

-(id)init {
    if (self = [super init]) {
        self.logFeatures = NO;
    }
    return self;
}

// The overrides
-(void)onRenderLayer:(MLNMapView *)mapView
       renderEncoder:(id<MTLRenderCommandEncoder>)renderEncoder {
   //NSLog(@"PluginLayerExample: On Render Layer");

}

-(void)onUpdateLayer {

}

-(void)onUpdateLayerProperties:(NSDictionary *)layerProperties {
  //  NSLog(@"Layer Properties: %@", layerProperties);
}

-(void)featureLoaded:(MLNPluginLayerTileFeature *)tileFeature {

    // Writing a single string since the tile loading is multithreaded and the output can get interwoven
    NSMutableString *outputStr = [NSMutableString string];
    [outputStr appendFormat:@"Tile Feature (id:%@) Properties: %@\n", tileFeature.featureID, tileFeature.featureProperties];

    for (NSValue *v in tileFeature.featureCoordinates) {

        CLLocationCoordinate2D coord;
        [v getValue:&coord];

        [outputStr appendFormat:@"  -> (%f, %f) \n", coord.latitude, coord.longitude];

    }

    NSLog(@"Feature: %@", outputStr);
}

-(void)featureUnloaded:(MLNPluginLayerTileFeature *)tileFeature {
   // NSLog(@"Tile Features Unloaded: %@", tileFeature.featureProperties);

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
