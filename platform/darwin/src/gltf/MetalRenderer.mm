//
//  GLTFRenderer.cpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/19/24.
//

#include "MetalRenderer.hpp"
#include <simd/simd.h>
#include <iostream>

#import "GLTFModelLoader.h"
#import "gltfkit/GLTFAnimation.h"
#import "gltfkit/GLTFMTLShaderBuilder.h"
#import "gltfkit/GLTFMTLBufferAllocator.h"


using namespace maplibre::gltf;

MetalRenderer::MetalRenderer() {
    GLTFRenderer();
}

MetalRenderer::~MetalRenderer() {
    std::cout << "Destructor\n";
}


void MetalRenderer::setMetalDevice(id<MTLDevice> device) {
    if (_metalDevice == device) {
        return;
    }

    _metalDevice = device;
    setupMetal();
}

// Set the drawable size
void MetalRenderer::setDrawableSize(int width, int height) {
    if ((_drawableSize.x == width) && (_drawableSize.y == height)) {
        return;
    }
    _drawableSize = {width, height};
    // updateFramebufferSize();
}

// Set the current drawable
//void MetalRenderer::setCurrentDrawable(id<CAMetalDrawable> drawable) {
//    _currentDrawable = drawable;
//}

// Set the output render pass descriptor (this is for outputting to a metal view)
//void MetalRenderer::setCurrentOutputRenderPassDescriptor(MTLRenderPassDescriptor *renderPassDescriptor) {
//    _currentOutputRenderPassDescriptor = renderPassDescriptor;
//}



// Update any animations
void MetalRenderer::update(float timeSinceLastDraw) {

    // Update the global time
    _globalTime += timeSinceLastDraw;

    // If we don't have an asset, then bail
    if (_models.size() == 0) {
        return;
    }

    double maxAnimDuration = 0;
    for (auto m: _models) {
        for (GLTFAnimation *animation in m->_asset.animations) {
            for (GLTFAnimationChannel *channel in animation.channels) {
                if (channel.duration > maxAnimDuration) {
                    maxAnimDuration = channel.duration;
                }
            }
        }
    }

    double animTime = fmod(_globalTime, maxAnimDuration);
    for (auto m: _models) {
        for (GLTFAnimation *animation in m->_asset.animations) {
            [animation runAtTime:animTime];
        }
    }

    _camera->updateWithTimestep(timeSinceLastDraw);
    for (auto m: _models) {
        computeTransforms(m);
    }

}

// Render
void MetalRenderer::render() {

    auto environmentMVP  = _metalRenderingEnvironment->_currentProjectionMatrix;
    double tileSize = 256.0;
    double zoom = _metalRenderingEnvironment->_currentZoomLevel;
    double scaleFactor = (20037508.34); // M_PI
    double worldSize = (tileSize / scaleFactor) * pow(2.0, zoom);
    simd_double4x4 scaleMatrix = GLTFMatrixFromScaleD(simd_make_double3(worldSize, -worldSize, 1.0));
    simd_double4x4 xlateMatrix = GLTFMatrixFromTranslationD(simd_make_double3(20037508.34,-20037508.34,0.0));

    auto m1 = matrix_multiply(scaleMatrix, xlateMatrix);
    auto m2 = matrix_multiply(environmentMVP, m1);
    _projectionMatrix = m2;

    // id <MTLCommandBuffer> internalCommandBuffer = [_internalMetalCommandQueue commandBuffer];
//
//    if (_existingCommandBuffer) {
//        commandBuffer = _metalRenderingEnvironment->_currentCommandBuffer;
//    } else {
//        // Create a command buffer and we're going to be expected to draw to the output
//        commandBuffer = [_internalMetalCommandQueue commandBuffer];
//        currentDrawable = _metalRenderingEnvironment->_currentDrawable;
//    }
//
    encodeMainPass(_metalRenderingEnvironment->_currentCommandBuffer);
    // if (_useBloomPass) {
    //     encodeBloomPasses(internalCommandBuffer);
    // }

//    [internalCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
//        dispatch_async(dispatch_get_main_queue(), ^{
//            signalFrameCompletion();
//        });
//    }];


    // This will write out the tone mapping
    //id<MTLCommandBuffer> externalBuffer = _metalRenderingEnvironment->_currentCommandBuffer;
    //encodeTonemappingPass(externalBuffer);
}

// Load a model
void MetalRenderer::loadGLTFModel(std::shared_ptr<GLTFModel> model) {

    GLTFModelLoader *modelLoader = [[GLTFModelLoader alloc] init];
    NSURL *u = [NSURL URLWithString:[NSString stringWithCString:model->_modelURL.c_str()]];
    [modelLoader loadURL:u
   withCompletionHandler:^(GLTFAsset * _Nonnull asset) {
        addGLTFAsset(asset, model);
    }
         bufferAllocator:_bufferAllocator];

}

// Set the rendering environemnt variables
void MetalRenderer::setRenderingEnvironemnt(std::shared_ptr<GLTFManagerRenderingEnvironment> renderingEnvironment) {
    GLTFRenderer::setRenderingEnvironemnt(renderingEnvironment);

    _metalRenderingEnvironment = std::static_pointer_cast<GLTFManagerRenderingEnvironmentMetal>(renderingEnvironment);

}


// RENDERING
void MetalRenderer::encodeMainPass(id<MTLCommandBuffer> commandBuffer) {

    id <MTLRenderCommandEncoder> renderEncoder = _metalRenderingEnvironment->_currentCommandEncoder;

    for (auto m: _models) {
        [renderEncoder pushDebugGroup:@"Draw glTF Scene"];
        renderScene(m, m->_asset.defaultScene, commandBuffer, renderEncoder);
        [renderEncoder popDebugGroup];
    }


}

void MetalRenderer::encodeBloomPasses(id<MTLCommandBuffer> commandBuffer) {

    MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
    pass.colorAttachments[0].texture = _bloomTextureA;
    pass.colorAttachments[0].loadAction = MTLLoadActionDontCare;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;

    id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:pass];
    [renderEncoder pushDebugGroup:@"Post-process (Bloom threshold)"];
    drawFullscreenPassWithPipeline(_bloomThresholdPipelineState,renderEncoder,_colorTexture);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

    pass = [MTLRenderPassDescriptor renderPassDescriptor];
    pass.colorAttachments[0].texture = _bloomTextureB;
    pass.colorAttachments[0].loadAction = MTLLoadActionDontCare;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:pass];
    [renderEncoder pushDebugGroup:@"Post-process (Bloom blur - horizontal)"];
    drawFullscreenPassWithPipeline(_blurHorizontalPipelineState,renderEncoder,_bloomTextureA);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

    pass = [MTLRenderPassDescriptor renderPassDescriptor];
    pass.colorAttachments[0].texture = _bloomTextureA;
    pass.colorAttachments[0].loadAction = MTLLoadActionDontCare;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:pass];
    [renderEncoder pushDebugGroup:@"Post-process (Bloom blur - vertical)"];
    drawFullscreenPassWithPipeline(_blurVerticalPipelineState,renderEncoder,_bloomTextureB);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

    pass = [MTLRenderPassDescriptor renderPassDescriptor];
    pass.colorAttachments[0].texture = _colorTexture;
    pass.colorAttachments[0].loadAction = MTLLoadActionLoad;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;

    renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:pass];
    [renderEncoder pushDebugGroup:@"Post-process (Bloom combine)"];
    drawFullscreenPassWithPipeline(_additiveBlendPipelineState,renderEncoder,_bloomTextureA);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];
}

void MetalRenderer::encodeTonemappingPass(id<MTLCommandBuffer> commandBuffer) {

    if (_metalRenderingEnvironment->_currentRenderPassDescriptor == nil) {
        return;
    }

    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:_metalRenderingEnvironment->_currentRenderPassDescriptor];
    [renderEncoder pushDebugGroup:@"Post-process (Tonemapping)"];
    drawFullscreenPassWithPipeline(_tonemapPipelineState,renderEncoder,_colorTexture);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];



}

void MetalRenderer::drawFullscreenPassWithPipeline(id<MTLRenderPipelineState> renderPipelineState,
                                                   id<MTLRenderCommandEncoder> renderCommandEncoder,
                                                   id<MTLTexture> sourceTexture) {
    float triangleData[] = {
        -1,  3, 0, -1,
        -1, -1, 0,  1,
         3, -1, 2,  1
    };
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    [renderCommandEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [renderCommandEncoder setCullMode:MTLCullModeNone];
    [renderCommandEncoder setVertexBytes:triangleData length:sizeof(float) * 12 atIndex:0];
    [renderCommandEncoder setFragmentTexture:sourceTexture atIndex:0];
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
}

void MetalRenderer::signalFrameCompletion() {
    dispatch_semaphore_signal(_frameBoundarySemaphore);
}


MTLRenderPassDescriptor* MetalRenderer::newRenderPassDescriptor() {
    MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
    if (_sampleCount > 1) {
        pass.colorAttachments[0].texture = _multisampleColorTexture;
        pass.colorAttachments[0].resolveTexture = _metalRenderingEnvironment->_colorTexture;
        pass.colorAttachments[0].loadAction = MTLLoadActionLoad;
        pass.colorAttachments[0].storeAction = MTLStoreActionMultisampleResolve;
        pass.colorAttachments[0].clearColor = MTLClearColorMake(1, 0, 0, 0);
    } else {
        pass.colorAttachments[0].texture = _metalRenderingEnvironment->_colorTexture;
        pass.colorAttachments[0].loadAction = MTLLoadActionLoad;
        pass.colorAttachments[0].storeAction = MTLStoreActionStore;
        pass.colorAttachments[0].clearColor = MTLClearColorMake(1, 0, 0, 0);

    }
    pass.depthAttachment.texture = _metalRenderingEnvironment->_depthStencilTexture;
    pass.depthAttachment.loadAction = MTLLoadActionLoad;
    pass.depthAttachment.storeAction = MTLStoreActionStore;

    return pass;
}

/*
 ------- ------- ----- -----------------------------------------------
 PRIVATE METHODS BELOW
 ------- ------- ----- -----------------------------------------------
*/


void MetalRenderer::setupMetal() {

    _internalMetalCommandQueue = [_metalDevice newCommandQueue];
    _metalLibrary = [_metalDevice newDefaultLibrary];

    _projectionMatrix = matrix_identity_double4x4;
    _colorPixelFormat = MTLPixelFormatBGRA8Unorm;
   _depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    _sampleCount = 1;
    _drawableSize = {1, 1};

    _frameBoundarySemaphore = dispatch_semaphore_create(_maxInflightFrames);

    loadBloomPipelines();
    loadTonemapPipeline();
    loadDepthClearPipeline();

    _opaqueRenderItems = [NSMutableArray array];
    _transparentRenderItems = [NSMutableArray array];
    _currentLightNodes = [NSMutableArray array];
    _deferredReusableBuffers = [NSMutableArray array];
    _bufferPool = [NSMutableArray array];

    _pipelineStatesForSubmeshes = [NSMutableDictionary dictionary];
    _depthStencilStateMap = [NSMutableDictionary dictionary];
    _texturesForImageIdentifiers = [NSMutableDictionary dictionary];
    _samplerStatesForSamplers = [NSMutableDictionary dictionary];

    _textureLoader = [[GLTFMTLTextureLoader alloc] initWithDevice:_metalDevice];
    _bufferAllocator = [[GLTFMTLBufferAllocator alloc] initWithDevice:_metalDevice];
    //_assets = [NSMutableArray array];




    MTLDepthStencilDescriptor *fullscreenTransfterDepthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
    fullscreenTransfterDepthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways; // Or another value as needed
    fullscreenTransfterDepthStencilDescriptor.depthWriteEnabled = NO;
    _fullscreenTransfterDepthStencilState = [_metalDevice newDepthStencilStateWithDescriptor:fullscreenTransfterDepthStencilDescriptor];



}

void MetalRenderer::updateFramebufferSize() {

    MTLTextureDescriptor *textureDescriptor = [MTLTextureDescriptor new];
    textureDescriptor.width = _drawableSize.x;
    textureDescriptor.height = _drawableSize.y;
    textureDescriptor.depth = 1;

    textureDescriptor.textureType = _sampleCount > 1 ? MTLTextureType2DMultisample : MTLTextureType2D;
    textureDescriptor.pixelFormat = _colorPixelFormat;
    textureDescriptor.sampleCount = _sampleCount;
    textureDescriptor.storageMode = MTLStorageModePrivate;
    textureDescriptor.usage = MTLTextureUsageRenderTarget;
    _multisampleColorTexture = [_metalDevice newTextureWithDescriptor:textureDescriptor];

    textureDescriptor.textureType = MTLTextureType2D;
    textureDescriptor.pixelFormat = _colorPixelFormat;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.storageMode = MTLStorageModePrivate;
    textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    _colorTexture = [_metalDevice newTextureWithDescriptor:textureDescriptor];

   textureDescriptor.textureType = _sampleCount > 1 ? MTLTextureType2DMultisample : MTLTextureType2D;
   textureDescriptor.pixelFormat = _depthStencilPixelFormat;
   textureDescriptor.sampleCount = _sampleCount;
   textureDescriptor.storageMode = MTLStorageModePrivate;
   textureDescriptor.usage = MTLTextureUsageRenderTarget;
   _depthStencilTexture = [_metalDevice newTextureWithDescriptor:textureDescriptor];

    textureDescriptor.width = _drawableSize.x / 2;
    textureDescriptor.height = _drawableSize.y / 2;
    textureDescriptor.textureType = MTLTextureType2D;
    textureDescriptor.pixelFormat = _colorPixelFormat;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.storageMode = MTLStorageModePrivate;
    textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    _bloomTextureA = [_metalDevice newTextureWithDescriptor:textureDescriptor];

    textureDescriptor.width = _drawableSize.x / 2;
    textureDescriptor.height = _drawableSize.y / 2;
    textureDescriptor.textureType = MTLTextureType2D;
    textureDescriptor.pixelFormat = _colorPixelFormat;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.storageMode = MTLStorageModePrivate;
    textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    _bloomTextureB = [_metalDevice newTextureWithDescriptor:textureDescriptor];

    //self.renderer.sampleCount = self.sampleCount;


}






void MetalRenderer::loadBloomPipelines() {

    NSError *error = nil;
    MTLRenderPipelineDescriptor *descriptor = [MTLRenderPipelineDescriptor new];
    descriptor.vertexFunction = [_metalLibrary newFunctionWithName:@"quad_vertex_main"];
    descriptor.colorAttachments[0].pixelFormat = _colorPixelFormat;
    descriptor.fragmentFunction = [_metalLibrary newFunctionWithName:@"bloom_threshold_fragment_main"];
    _bloomThresholdPipelineState = [_metalDevice newRenderPipelineStateWithDescriptor:descriptor error:&error];
    if (_bloomThresholdPipelineState == nil) {
        NSLog(@"Error occurred when creating render pipeline state: %@", error);
    }

    descriptor.fragmentFunction = [_metalLibrary newFunctionWithName:@"blur_horizontal7_fragment_main"];
    _blurHorizontalPipelineState = [_metalDevice newRenderPipelineStateWithDescriptor:descriptor error:&error];
    if (_blurHorizontalPipelineState == nil) {
        NSLog(@"Error occurred when creating render pipeline state: %@", error);
    }

    descriptor.fragmentFunction = [_metalLibrary newFunctionWithName:@"blur_vertical7_fragment_main"];
    _blurVerticalPipelineState = [_metalDevice newRenderPipelineStateWithDescriptor:descriptor error:&error];
    if (_blurVerticalPipelineState == nil) {
        NSLog(@"Error occurred when creating render pipeline state: %@", error);
    }

    descriptor.fragmentFunction = [_metalLibrary newFunctionWithName:@"additive_blend_fragment_main"];

    // Original Values
    descriptor.colorAttachments[0].blendingEnabled = YES;
    descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
    descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
    descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorZero;

    descriptor.colorAttachments[0].blendingEnabled = YES;
    descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationMin;
    descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
    descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
    descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorDestinationAlpha;


    _additiveBlendPipelineState = [_metalDevice newRenderPipelineStateWithDescriptor:descriptor error:&error];
    if (_additiveBlendPipelineState == nil) {
        NSLog(@"Error occurred when creating render pipeline state: %@", error);
    }
}

void MetalRenderer::loadTonemapPipeline() {

    NSError *error = nil;
    MTLRenderPipelineDescriptor *descriptor = [MTLRenderPipelineDescriptor new];


    // MT: Added blending
    descriptor.colorAttachments[0].blendingEnabled = YES;
    descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;


    id <MTLFunction> vertexFunction = [_metalLibrary newFunctionWithName:@"quad_vertex_main"];

    descriptor.vertexFunction = vertexFunction;
    descriptor.fragmentFunction = [_metalLibrary newFunctionWithName:@"tonemap_fragment_main"];
    descriptor.sampleCount = _sampleCount;
    descriptor.colorAttachments[0].pixelFormat = _colorPixelFormat;

    // TODO: This was hard coded to fix an issue whe moving to ML.  Need to sort out
    // how these params are set in ML and pass them into here
    descriptor.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    descriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;


    _tonemapPipelineState = [_metalDevice newRenderPipelineStateWithDescriptor:descriptor error:&error];
    if (_tonemapPipelineState == nil) {
        NSLog(@"Error occurred when creating render pipeline state: %@", error);
    }
}

void MetalRenderer::loadDepthClearPipeline() {
    MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];

    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
    depthStencilDescriptor.depthWriteEnabled = YES;
    depthStencilDescriptor.label = @"depthStencilClear";
    _depthStencilClearState = [_metalDevice newDepthStencilStateWithDescriptor:depthStencilDescriptor];

    MTLRenderPipelineDescriptor *renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];

    renderPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    renderPipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    renderPipelineDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    renderPipelineDescriptor.rasterSampleCount = _sampleCount;

    renderPipelineDescriptor.vertexFunction = [_metalLibrary newFunctionWithName:@"vertex_depth_clear"];
    renderPipelineDescriptor.vertexFunction.label = @"vertexDepthClear";

    renderPipelineDescriptor.fragmentFunction = [_metalLibrary newFunctionWithName:@"fragment_depth_clear"];
    renderPipelineDescriptor.fragmentFunction.label = @"fragmentDepthClear";

    MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[0].offset = 0;
    vertexDescriptor.attributes[0].bufferIndex = 0;
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    vertexDescriptor.layouts[0].stride = 8;

    renderPipelineDescriptor.vertexDescriptor = vertexDescriptor;

    NSError* error = NULL;
    renderPipelineDescriptor.label = @"pipelineDepthClear";
    _pipelineDepthClearState = [_metalDevice newRenderPipelineStateWithDescriptor:renderPipelineDescriptor error:&error];
}

void MetalRenderer::clearDepth(id<MTLRenderCommandEncoder> renderEncoder) {
    [renderEncoder setDepthStencilState:_depthStencilClearState];
    [renderEncoder setRenderPipelineState:_pipelineDepthClearState];

    const float clearCoords[8] = {
        -1, -1,
        1, -1,
        -1, 1,
        1, 1
    };

    [renderEncoder setVertexBytes:clearCoords length:sizeof(float) * 8 atIndex:0];
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];

}

void MetalRenderer::enqueueReusableBuffer(id<MTLBuffer> buffer) {
    [_bufferPool addObject:buffer];
}

id<MTLRenderPipelineState> MetalRenderer::renderPipelineStateForSubmesh(GLTFSubmesh *submesh) {

    id<MTLRenderPipelineState> pipeline = _pipelineStatesForSubmeshes[submesh.identifier];

    if (pipeline == nil) {
        GLTFMTLShaderBuilder *shaderBuilder = [[GLTFMTLShaderBuilder alloc] init];
        pipeline = [shaderBuilder renderPipelineStateForSubmesh: submesh
                                               colorPixelFormat:_colorPixelFormat
                                        depthStencilPixelFormat:_depthStencilPixelFormat
                                                    sampleCount:_sampleCount
                                                         device:_metalDevice];
        _pipelineStatesForSubmeshes[submesh.identifier] = pipeline;
    }

    return pipeline;
}
