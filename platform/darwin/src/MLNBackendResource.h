#if MLN_RENDER_BACKEND_METAL

#import <MetalKit/MetalKit.h>

@interface MLNBackendResource : NSObject

@property(nonatomic, strong) MTKView *mtkView;
@property(nonatomic, strong) id<MTLDevice> device;
@property(nonatomic, strong) MTLRenderPassDescriptor *renderPassDescriptor;
@property(nonatomic, strong) id<MTLCommandBuffer> commandBuffer;

- (instancetype)initWithMTKView:(MTKView *)mtkView
                         device:(id<MTLDevice>)device
           renderPassDescriptor:(MTLRenderPassDescriptor *)renderPassDescriptor
                  commandBuffer:(id<MTLCommandBuffer>)commandBuffer;

@end

#else

typedef struct {
} MLNBackendResource;

#endif
