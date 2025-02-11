//
//  CustomLayerTest.h
//  MapLibreTest
//
//  Created by Malcolm Toon on 11/7/24.
//

#import "MLNCustomStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

@interface GLTFModelLayer : MLNCustomStyleLayer

// This sets the relative light position for all the models being rendered
// The light position is in meters from the origin of the model.
- (void)setLightPositionX:(float)x y:(float)y z:(float)z;

- (void)loadModelFromJSON:(NSString *)modelMetadataFilename;

- (void)loadModel:(NSString *)appResourceFilename
              lat:(double)lat
              lon:(double)lon
      rotationDeg:(double)rotationDeg
      scaleFactor:(float)scaleFactor
       brightness:(float)brightness;

@end

NS_ASSUME_NONNULL_END
