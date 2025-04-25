//
//  MLNPluginLayer.h
//  App
//
//  Created by Malcolm Toon on 4/23/25.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

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

@end






@interface MLNPluginLayer : NSObject

/// Returns the layer capabilities of the plugin layer.
/// This must be overridden by the plug-in layer and return a set of capabilities
-(MLNPluginLayerCapabilities *)layerCapabilities;


// Creates the layer
- (void)createLayerFromProperties:(NSDictionary *)properties;

@end

NS_ASSUME_NONNULL_END
