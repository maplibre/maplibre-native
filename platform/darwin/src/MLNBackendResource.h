#if MLN_RENDER_BACKEND_METAL

#import <MetalKit/MetalKit.h>

typedef struct {
    MTKView *mtkView;
    id<MTLDevice> device;
    MTLRenderPassDescriptor *renderPassDescriptor;
    id<MTLCommandBuffer> commandBuffer;
} MLNBackendResource;

#else

typedef struct {
} MLNBackendResource;

#endif
