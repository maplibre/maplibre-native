//
//  GLTFRenderer.hpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/19/24.
//

#ifndef MetalRenderer_hpp
#define MetalRenderer_hpp

#include <stdio.h>
#include <simd/simd.h>
#include "GLTFRenderer.hpp"
#import "gltfkit/GLTFAsset.h"
#import "gltfkit//GLTFNode.h"
#import "gltfkit/GLTFImage.h"
#import "gltfkit/GLTFTextureSampler.h"
#include "GLTFMTLRenderItem.h"
#import "gltfkit/GLTFMTLTextureLoader.h"
#include "GLTFManagerRenderingEnvironmentMetal.hpp"
#include <QuartzCore/CAMetalLayer.h>
#include <Metal/Metal.h>
#include <vector>

namespace maplibre {
namespace gltf {

class GLTFRenderModel {
public:
    // The model metadata
    std::shared_ptr<GLTFModel> _gltfModel = nullptr;

    // The loaded model
    GLTFAsset *_asset = nil;

    // Rendering variables
    // Translation Vector is the translation of the model from the
    simd_float3 _translationVector; // This might need to be a doubles vector
    double _rotationDeg = 0;        // Rotation around the zAxis (if applicable)
    double _scaling = 1;            // Scaling from
    float _brightness = 1.0;        // Brightness

    // This is the model matrix (rotation, scaling and transformation applied)
    simd_double4x4 _modelMatrix;

    // This is the model matrix combined with the camera's view matrix
    simd_double4x4 _modelViewMatrix;

    // We probably won't need this in the final thing.  Used to normalize the model to the
    // viewport size
    simd_double4x4 _regularizationMatrix;

    //
    GLTFBoundingSphere _boundingSphere;
};

//
class MetalRenderer : public GLTFRenderer {
public:
    // Constructor
    MetalRenderer();
    ~MetalRenderer();

    void setMetalDevice(id<MTLDevice> device);

    // Set the drawable size
    void setDrawableSize(int width, int height) override;

    // Set the current drawable (used for a metal view)
    // void setCurrentDrawable(id<CAMetalDrawable> drawable);

    // Set the output render pass descriptor (this is for outputting to a metal view)
    // void setCurrentOutputRenderPassDescriptor(MTLRenderPassDescriptor *renderPassDescriptor);

    // Update any animations
    void update(float timeSinceLastDraw) override;

    // Render
    void render() override;

    // Load a model
    void loadGLTFModel(std::shared_ptr<GLTFModel> model) override;

    // Set the rendering environemnt variables
    void setRenderingEnvironemnt(std::shared_ptr<GLTFManagerRenderingEnvironment> renderingEnvironment) override;

private:
    // Rendering environment variables
    simd_double4x4 _projectionMatrix;
    int _sampleCount = 1;
    MTLPixelFormat _colorPixelFormat;
    MTLPixelFormat _depthStencilPixelFormat;
    simd_int2 _drawableSize;
    double _globalTime = 0;
    int _maxInflightFrames = 3;

    // Current external environment variables
    std::shared_ptr<GLTFManagerRenderingEnvironmentMetal> _metalRenderingEnvironment = nullptr;

    // id<CAMetalDrawable> _currentDrawable = nullptr;
    // MTLRenderPassDescriptor *_currentOutputRenderPassDescriptor = nullptr;

    dispatch_semaphore_t _frameBoundarySemaphore;

    void encodeMainPass(id<MTLCommandBuffer> commandBuffer);
    void encodeBloomPasses(id<MTLCommandBuffer> commandBuffer);
    void encodeTonemappingPass(id<MTLCommandBuffer> commandBuffer);
    void drawFullscreenPassWithPipeline(id<MTLRenderPipelineState> renderPipelineState,
                                        id<MTLRenderCommandEncoder> renderCommandEncoder,
                                        id<MTLTexture> sourceTexture);
    void signalFrameCompletion();
    MTLRenderPassDescriptor *newRenderPassDescriptor();

private:
    // Per-frame
    void computeTransforms(std::shared_ptr<GLTFRenderModel> model);

protected:
    void loadBloomPipelines() override;
    void loadTonemapPipeline() override;
    void updateFramebufferSize() override; // Updates the framebuffers based on size

private:
    // Set by the environment
    id<MTLDevice> _metalDevice = nullptr;

    // Created for rendering the model
    id<MTLCommandQueue> _internalMetalCommandQueue = nullptr;
    id<MTLLibrary> _metalLibrary = nullptr;

    // Setup the rendering environment.  Called after device is set
    void setupMetal();

    // Bloom
    id<MTLRenderPipelineState> _bloomThresholdPipelineState = nullptr;
    id<MTLRenderPipelineState> _blurHorizontalPipelineState = nullptr;
    id<MTLRenderPipelineState> _blurVerticalPipelineState = nullptr;
    id<MTLRenderPipelineState> _additiveBlendPipelineState = nullptr;

    // Tonemap
    id<MTLRenderPipelineState> _tonemapPipelineState = nullptr;

    // Render targets
    id<MTLTexture> _multisampleColorTexture = nullptr;
    id<MTLTexture> _colorTexture = nullptr;
    id<MTLTexture> _depthStencilTexture = nullptr;
    id<MTLTexture> _bloomTextureA = nullptr;
    id<MTLTexture> _bloomTextureB = nullptr;

    // Depth Clear
    id<MTLDepthStencilState> _depthStencilClearState = nullptr;
    id<MTLRenderPipelineState> _pipelineDepthClearState = nullptr;

    void loadDepthClearPipeline();
    void clearDepth(id<MTLRenderCommandEncoder>);

public:
    // Encapsulated in GLTFRenderer+GLTFAsset
    void setGLTFAsset(GLTFAsset *asset, std::shared_ptr<GLTFModel> model);

    //
    void addGLTFAsset(GLTFAsset *asset, std::shared_ptr<GLTFModel> model);

    //
    id<GLTFBufferAllocator> _bufferAllocator;

private:
    std::shared_ptr<GLTFRenderModel> createRenderModel(GLTFAsset *asset, std::shared_ptr<GLTFModel> model);

    std::vector<std::shared_ptr<GLTFRenderModel>> _models;
    //        GLTFAsset *_asset = nil;
    //        NSMutableArray *_assets = nil;

    //
    void computeRegularizationMatrix(std::shared_ptr<GLTFRenderModel> model);
    void addDefaultLights(std::shared_ptr<GLTFRenderModel> model);

    void renderScene(std::shared_ptr<GLTFRenderModel> model,
                     GLTFScene *scene,
                     id<MTLCommandBuffer> commandBuffer,
                     id<MTLRenderCommandEncoder> renderEncoder);

    void buildLightListRecursive(GLTFNode *node);

    void buildRenderListRecursive(std::shared_ptr<GLTFRenderModel> model,
                                  GLTFNode *node,
                                  simd_float4x4 modelMatrix,
                                  GLTFKHRLight *defaultAmbientLight);

    void drawRenderList(NSArray<GLTFMTLRenderItem *> *renderList, id<MTLRenderCommandEncoder> renderEncoder);

    id<MTLRenderPipelineState> renderPipelineStateForSubmesh(GLTFSubmesh *submesh);

    void bindTexturesForMaterial(GLTFMaterial *material, id<MTLRenderCommandEncoder> renderEncoder);

    id<MTLBuffer> dequeueReusableBufferOfLength(size_t length);

    void computeJointsForSubmesh(GLTFSubmesh *submesh, GLTFNode *node, id<MTLBuffer> jointBuffer);

    id<MTLDepthStencilState> depthStencilStateForDepthWriteEnabled(bool depthWriteEnabled,
                                                                   bool depthTestEnabled,
                                                                   MTLCompareFunction compareFunction);

    id<MTLTexture> textureForImage(GLTFImage *image, bool sRGB);
    id<MTLSamplerState> samplerStateForSampler(GLTFTextureSampler *sampler);

    NSMutableArray<GLTFMTLRenderItem *> *_opaqueRenderItems;
    NSMutableArray<GLTFMTLRenderItem *> *_transparentRenderItems;
    NSMutableArray<GLTFNode *> *_currentLightNodes;
    NSMutableArray<id<MTLBuffer>> *_deferredReusableBuffers;
    NSMutableArray<id<MTLBuffer>> *_bufferPool;
    NSMutableDictionary<NSNumber *, id<MTLDepthStencilState>> *_depthStencilStateMap;

    NSMutableDictionary<NSUUID *, id<MTLRenderPipelineState>> *_pipelineStatesForSubmeshes;
    NSMutableDictionary<NSUUID *, id<MTLTexture>> *_texturesForImageIdentifiers;
    GLTFMTLTextureLoader *_textureLoader;
    NSMutableDictionary<GLTFTextureSampler *, id<MTLSamplerState>> *_samplerStatesForSamplers;

    void enqueueReusableBuffer(id<MTLBuffer> buffer);

    // This is the depth stencil descriptor for overlaying the
    id<MTLDepthStencilState> _fullscreenTransfterDepthStencilState = nullptr;
};

} // namespace gltf
} // namespace maplibre

#endif /* MetalRenderer_hpp */
