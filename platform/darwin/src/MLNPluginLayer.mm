#import "MLNPluginLayer.h"

@implementation MLNPluginLayerTileFeature

@end

@implementation MLNPluginLayerTileFeatureCollection

@end

@implementation MLNPluginLayerProperty

+(MLNPluginLayerProperty *)propertyWithName:(NSString *)propertyName
                               propertyType:(MLNPluginLayerPropertyType)propertyType
                               defaultValue:(id)defaultValue {
    MLNPluginLayerProperty *tempResult = [[MLNPluginLayerProperty alloc] init];
    tempResult.propertyName = propertyName;
    tempResult.propertyType = propertyType;

    if (propertyType == MLNPluginLayerPropertyTypeSingleFloat) {
        if ([defaultValue isKindOfClass:[NSNumber class]]) {
            tempResult.singleFloatDefaultValue = [defaultValue floatValue];
        }
    } else if (propertyType == MLNPluginLayerPropertyTypeColor) {
#if TARGET_OS_IPHONE
        if ([defaultValue isKindOfClass:[UIColor class]]) {
#else
        if ([defaultValue isKindOfClass:[NSColor class]]) {
#endif
            tempResult.colorDefaultValue = defaultValue;
        }
    }

    return tempResult;
}


-(id)init {
    // Base class implemenation
    if (self = [super init]) {
        // Default setup
        self.propertyType = MLNPluginLayerPropertyTypeUnknown;
        self.propertyName = @"unknown";

        // Default values for the various types
        self.singleFloatDefaultValue = 1.0;
    }
    return self;
}

@end

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

- (void)onUpdateLayer:(MLNPluginLayerDrawingContext)drawingContext {
    // Base class does nothing
}

-(void)onUpdateLayerProperties:(NSDictionary *)layerProperties {
    // Base class does nothing
}

// If the layer properties indicate that this layer has a the ability to intercept
// features, then this method will be called when a feature is loaded
- (void)onFeatureCollectionLoaded:(MLNPluginLayerTileFeatureCollection *)tileFeatureCollection {
    // Base class does nothing
}

/// Called when a set of features are unloaded because the tile goes out of scene/etc
- (void)onFeatureCollectionUnloaded:(MLNPluginLayerTileFeatureCollection *)tileFeatureCollection {
    // Base class does nothing
}


/// Added to a map view
- (void)didMoveToMapView:(MLNMapView *)mapView {
    // Base class does nothing
}



@end



@implementation MLNPluginLayerCapabilities
@end
