//
//  MLNPluginLayer.h
//  App
//
//  Created by Malcolm Toon on 4/23/25.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MLNPluginLayer : NSObject

// Returns the layer type of the plugin layer
-(NSString *)layerTypeID;

// Creates the layer
-(void)createLayerFromProperties:(NSDictionary *)properties;

@end

NS_ASSUME_NONNULL_END
