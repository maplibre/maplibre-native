//
//  MLNPluginLayer.h
//  App
//
//  Created by Malcolm Toon on 4/23/25.
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

NS_ASSUME_NONNULL_BEGIN

typedef enum {
  MLNPluginLayerPropertyTypeUnknown,
  MLNPluginLayerPropertyTypeSingleFloat
} MLNPluginLayerPropertyType;

@interface MLNPluginLayerProperty : NSObject

// The name of the property
@property NSString *propertyName;

// The type of property
@property MLNPluginLayerPropertyType propertyType;

// Single float default value
@property float singleFloatDefaultValue;

@end

typedef enum {
  MLNPluginLayerTileKindGeometry,
  MLNPluginLayerTileKindRaster,
  MLNPluginLayerTileKindRasterDEM,
  MLNPluginLayerTileKindNotRequired
} MLNPluginLayerTileKind;

@interface MLNPluginLayerCapabilities : NSObject

@property NSString *layerID;
@property BOOL requiresSource;
@property BOOL requiresPass3D;
@property BOOL requiresLayout;
@property BOOL requiresRenderingFadingTiles;
@property BOOL requiresCrossTileIndex;
@property MLNPluginLayerTileKind tileKind;

//! This is a list of layer properties that this layer supports.
@property NSArray<MLNPluginLayerProperty *> *layerProperties;

@end

@class MLNMapView;

@interface MLNPluginLayer : NSObject

/// Returns the layer capabilities of the plugin layer.
/// This must be overridden by the plug-in layer and return a set of capabilities
+ (MLNPluginLayerCapabilities *)layerCapabilities;

// These are public methods that can be overridden by the plugin layer
/// Called when the layer is rendered
- (void)onRenderLayer:(MLNMapView *)mapView
        renderEncoder:(id<MTLRenderCommandEncoder>)renderEncoder;

/// Called when the layer is updated in the render loop.  This would update animations/etc
- (void)onUpdateLayer;

/// Called when the layer properties are updated.  Can be on initial load from the JSON or when
/// dynamic properties are updated
- (void)onUpdateLayerProperties:(NSDictionary *)layerProperties;

/// Added to a map view
- (void)didMoveToMapView:(MLNMapView *)mapView;

@end

NS_ASSUME_NONNULL_END
