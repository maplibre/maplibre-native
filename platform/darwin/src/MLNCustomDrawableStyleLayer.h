#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNFoundation.h"
#import "MLNStyleValue.h"
#import "MLNStyleLayer.h"
#import "MLNGeometry.h"

#ifdef __cplusplus
#include <mbgl/style/layer.hpp>
#endif

@interface MLNCustomDrawableStyleLayer : MLNStyleLayer

#ifdef __cplusplus
- (instancetype)initWithPendingLayer:(std::unique_ptr<mbgl::style::Layer>)pendingLayer;
#endif

@end
