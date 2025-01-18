#import "MLNBackendResource.h"

@implementation MLNBackendResource

- (instancetype)initWithMTKView:(MTKView *)mtkView
                        device:(id<MTLDevice>)device
         renderPassDescriptor:(MTLRenderPassDescriptor *)renderPassDescriptor
               commandBuffer:(id<MTLCommandBuffer>)commandBuffer {
    self = [super init];
    if (self) {
        _mtkView = mtkView;
        _device = device;
        _renderPassDescriptor = renderPassDescriptor;
        _commandBuffer = commandBuffer;
    }
    return self;
}

@end