//
//  Copyright (c) 2018 Warren Moore. All rights reserved.
//
//  Permission to use, copy, modify, and distribute this software for any
//  purpose with or without fee is hereby granted, provided that the above
//  copyright notice and this permission notice appear in all copies.
//
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
//  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
//  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
//  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#import "GLTFAnimation.h"
#import "GLTFAccessor.h"
#import "GLTFBufferView.h"
#import "GLTFBuffer.h"
#import "GLTFNode.h"

@implementation GLTFAnimationSampler

- (NSString *)description {
    return [NSString stringWithFormat:@"%@ interpolation: %d", super.description, (int)self.interpolationMode];
}

- (const void *)inputValues {
    return [self.inputAccessor.bufferView.buffer contents] + self.inputAccessor.bufferView.offset + self.inputAccessor.offset;
}

- (const void *)outputValues {
    return [self.outputAccessor.bufferView.buffer contents] + self.outputAccessor.bufferView.offset + self.outputAccessor.offset;
}

- (int)keyFrameCount {
    return (int)self.inputAccessor.count;
}

@end

@implementation GLTFAnimationChannel

- (NSString *)description {
    return [NSString stringWithFormat:@"%@ target: %@; path: %@; sampler: %@", super.description, self.targetNode, self.targetPath, self.sampler];
}

- (NSTimeInterval)startTime {
    GLTFAnimationSampler *sampler = self.sampler;
    const float *timeValues = sampler.inputValues;
    float startTime = timeValues[0];
    return startTime;
}

- (NSTimeInterval)endTime {
    GLTFAnimationSampler *sampler = self.sampler;
    const float *timeValues = sampler.inputValues;
    int keyFrameCount = sampler.keyFrameCount;
    float endTime = timeValues[keyFrameCount - 1];
    return endTime;
}

- (NSTimeInterval)duration {
    return self.endTime - self.startTime;
}

@end

@implementation GLTFAnimation

- (NSString *)description {
    return [NSString stringWithFormat:@"%@ channels: %@", super.description, self.channels];
}

- (void)runAtTime:(NSTimeInterval)time {
    for (GLTFAnimationChannel *channel in self.channels) {
        GLTFNode *target = channel.targetNode;
        NSString *path = channel.targetPath;
        GLTFAnimationSampler *sampler = channel.sampler;
        
        int keyFrameCount = sampler.keyFrameCount;
        
        const float *timeValues = sampler.inputValues;
        
        float minTime = timeValues[0];
        float maxTime = timeValues[keyFrameCount - 1];

        if (time < minTime || time > maxTime) {
            continue;
        }
        
        int previousKeyFrame = 0, nextKeyFrame = 1;
        while (timeValues[nextKeyFrame] < time) {
            ++previousKeyFrame;
            ++nextKeyFrame;
        }
        
        if (previousKeyFrame >= keyFrameCount) {
            previousKeyFrame = 0;
        }
        
        if (nextKeyFrame >= keyFrameCount) {
            nextKeyFrame = 0;
        }
        
        float frameTimeDelta = timeValues[nextKeyFrame] - timeValues[previousKeyFrame];
        float timeWithinFrame = time - timeValues[previousKeyFrame];
        float frameProgress = timeWithinFrame / frameTimeDelta;
        
        if ([path isEqualToString:@"rotation"]) {
            if(sampler.outputAccessor.componentType != GLTFDataTypeFloat) {
                static dispatch_once_t floatRotationsNonce;
                dispatch_once(&floatRotationsNonce, ^{
                    NSLog(@"WARNING: Only float accessors are supported for rotation animations. This will only be reported once.");
                });
            }
            const GLTFQuaternion *rotationValues = sampler.outputValues;
            
            GLTFQuaternion previousRotation = rotationValues[previousKeyFrame];
            GLTFQuaternion nextRotation = rotationValues[nextKeyFrame];
            GLTFQuaternion interpRotation = simd_slerp(previousRotation, nextRotation, frameProgress);

            target.rotationQuaternion = interpRotation;
        } else if ([path isEqualToString:@"translation"]) {
            const GLTFVector3 *translationValues = sampler.outputValues;
            
            GLTFVector3 previousTranslation = translationValues[previousKeyFrame];
            GLTFVector3 nextTranslation = translationValues[nextKeyFrame];
            
            GLTFVector3 interpTranslation = (GLTFVector3) {
                ((1 - frameProgress) * previousTranslation.x) + (frameProgress * nextTranslation.x),
                ((1 - frameProgress) * previousTranslation.y) + (frameProgress * nextTranslation.y),
                ((1 - frameProgress) * previousTranslation.z) + (frameProgress * nextTranslation.z)
            };

            target.translation = (simd_float3){ interpTranslation.x, interpTranslation.y, interpTranslation.z };
        } else if ([path isEqualToString:@"scale"]) {
            const float *scaleValues = sampler.outputValues;
            
            float previousScale = scaleValues[previousKeyFrame];
            float nextScale = scaleValues[nextKeyFrame];
            
            float interpScale = ((1 - frameProgress) * previousScale) + (frameProgress * nextScale);
            
            target.scale = (simd_float3)interpScale;
        } else if ([path isEqualToString:@"weights"]) {
            if(sampler.outputAccessor.componentType != GLTFDataTypeFloat) {
                static dispatch_once_t floatWeightsNonce;
                dispatch_once(&floatWeightsNonce, ^{
                    NSLog(@"WARNING: Only scalar float accessors are supported for weight animations. This will only be reported once.");
                });
            }
            const float *weightValues = sampler.outputValues;
            
            long weightCount = sampler.outputAccessor.count / keyFrameCount;
            
            const float *previousWeights = weightValues + (previousKeyFrame * weightCount);
            const float *nextWeights = weightValues + (nextKeyFrame * weightCount);
            
            NSMutableArray *interpWeights = [NSMutableArray array];
            for (int i = 0; i < weightCount; ++i) {
                float interpWeight = ((1 - frameProgress) * previousWeights[i]) + (frameProgress * nextWeights[i]);
                [interpWeights addObject:@(interpWeight)];
            }

            target.morphTargetWeights = [interpWeights copy];
        }
    }
}

@end
