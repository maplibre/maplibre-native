//
//  MLNPLuginStyleLayer.m
//  App
//
//  Created by Malcolm Toon on 5/17/25.
//

#import "MLNPLuginStyleLayer.h"
#import "MLNPLuginStyleLayer_Private.h"

@implementation MLNPLuginStyleLayer

@end


MLNStyleLayer* mbgl::PluginLayerPeerFactory::createPeer(style::Layer *rawLayer) {
    return [[MLNPLuginStyleLayer alloc] initWithRawLayer:rawLayer];

}
