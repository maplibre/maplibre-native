//
//  CustomLayerTest.h
//  MapLibreTest
//
//  Created by Malcolm Toon on 11/7/24.
//

#import "MLNCustomStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

@interface GLTFModelLayer : MLNCustomStyleLayer

-(void)loadModel:(NSString *)appResourceFilename
             lat:(double)lat
             lon:(double)lon
     rotationDeg:(double)rotationDeg;

@end

NS_ASSUME_NONNULL_END
