//
//  Untitled.h
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/19/24.
//

#include "MetalRenderer.hpp"


#import "GLTFMath.hpp"
#import "gltfkit/GLTFScene.h"
#import "gltfkit/GLTFMesh.h"
#import "gltfkit/GLTFMaterial.h"
#import "gltfkit/GLTFKHRLight.h"
#import "gltfkit/GLTFMTLUtilities.h"
#import "gltfkit/GLTFSkin.h"
#import "gltfkit/GLTFMTLBufferAllocator.h"
#import "gltfkit/GLTFAccessor.h"
#import "gltfkit/GLTFBufferView.h"
#import "gltfkit/GLTFVertexDescriptor.h"



using namespace maplibre::gltf;

// Encapsulated in GLTFRenderer+GLTFAsset
void MetalRenderer::setGLTFAsset(GLTFAsset *asset, std::shared_ptr<GLTFModel> model) {
    
    _models.clear();
    addGLTFAsset(asset, model);
    
//    _asset = asset;
//    if (_asset != nullptr) {
//        computeRegularizationMatrix();
//        computeTransforms();
//        addDefaultLights();
//    }

    
}

//
void MetalRenderer::addGLTFAsset(GLTFAsset *asset, std::shared_ptr<GLTFModel> model) {
    
    auto m = createRenderModel(asset, model);
    _models.push_back(m);
    
}

std::shared_ptr<GLTFRenderModel> MetalRenderer::createRenderModel(GLTFAsset *asset, std::shared_ptr<GLTFModel> model) {
    
    std::shared_ptr<GLTFRenderModel> tempResult = std::make_shared<GLTFRenderModel>();
    tempResult->_asset = asset;
    tempResult->_gltfModel = model;
    tempResult->_scaling = model->_scaleFactor;
    tempResult->_brightness = model->_brightness;

    computeRegularizationMatrix(tempResult);
    computeTransforms(tempResult);
    addDefaultLights(tempResult);

    return tempResult;
    
}



void MetalRenderer::addDefaultLights(std::shared_ptr<GLTFRenderModel> model) {
//    GLTFNode *lightNode = [[GLTFNode alloc] init];
//    lightNode.translation = (simd_float3){ 0, 0, 1 };
//    lightNode.rotationQuaternion = simd_quaternion(1.0f, 0, 0, 0);
//    GLTFKHRLight *light = [[GLTFKHRLight alloc] init];
//    lightNode.light = light;
//    [self.asset.defaultScene addNode:lightNode];
//    [self.asset addLight:light];
//
//    GLTFKHRLight *ambientLight = [[GLTFKHRLight alloc] init];
//    ambientLight.type = GLTFKHRLightTypeAmbient;
//    ambientLight.intensity = 0.1;
//    [self.asset addLight:ambientLight];
//    self.asset.defaultScene.ambientLight = ambientLight;
}

void MetalRenderer::computeRegularizationMatrix(std::shared_ptr<GLTFRenderModel> model) {
    GLTFBoundingSphere bounds = GLTFBoundingSphereFromBox(model->_asset.defaultScene.approximateBounds);
    model->_boundingSphere = bounds;
    float scale = (bounds.radius > 0) ? (1 / (bounds.radius)) : 1;
    simd_double4x4 centerScale = GLTFMatrixFromUniformScaleD(scale);
    simd_double4x4 centerTranslation = GLTFMatrixFromTranslationD(-bounds.center);
    
    // This regularization matrix centers the model
    model->_regularizationMatrix = matrix_multiply(centerScale, centerTranslation);
    
    
    // The regularization matrix just scales it to show in the viewport
    model->_regularizationMatrix = centerScale;
    
}

void MetalRenderer::computeTransforms(std::shared_ptr<GLTFRenderModel> model) {
    // New stuff
    auto mdlMatrix = matrix_identity_double4x4;

    // Rotation
    auto modelRotation = GLTFRotationMatrixFromAxisAngleD(GLTFAxisYD, (model->_gltfModel->_rotationDeg) * DEG_RAD);
    auto modelRotated = matrix_multiply(modelRotation, mdlMatrix);
    
    // Tilt
    auto modelTilted = GLTFRotationMatrixFromAxisAngleD(GLTFAxisXD, 90 * DEG_RAD);
    modelTilted = matrix_multiply(modelTilted, modelRotated);

    simd_double3 xlateVector = simd_make_double3(model->_gltfModel->_xLateX,
                                                    model->_gltfModel->_xLateY,
                                                    model->_gltfModel->_xLateZ);
    
    auto xLateMatrix = GLTFMatrixFromTranslationD(xlateVector);

    auto modelTranslated = matrix_multiply(xLateMatrix, modelTilted);

    model->_modelMatrix = modelTranslated;

    model->_modelViewMatrix = modelTranslated;

    return;
}

void MetalRenderer::renderScene(std::shared_ptr<GLTFRenderModel> model,
                                GLTFScene *scene,
                                id<MTLCommandBuffer> commandBuffer,
                                id<MTLRenderCommandEncoder> renderEncoder) {
    
    if (scene == nil) {
        return;
    }
/*
    long timedOut = dispatch_semaphore_wait(_frameBoundarySemaphore, dispatch_time(0, 1 * NSEC_PER_SEC));
    if (timedOut) {
        NSLog(@"Failed to receive frame boundary signal before timing out; calling signalFrameCompletion manually. "
              "Remember to call signalFrameCompletion on GLTFMTLRenderer from the completion handler of the command buffer "
              "into which you encode the work for drawing assets");
        signalFrameCompletion();
    }
    */
    //self.ambientLight = scene.ambientLight;

    for (GLTFNode *rootNode in scene.nodes) {
        buildLightListRecursive(rootNode);
    }
    
    for (GLTFNode *rootNode in scene.nodes) {
        buildRenderListRecursive(model,
                                 rootNode, matrix_identity_float4x4, scene.ambientLight);
    }
    
    NSMutableArray *renderList = [NSMutableArray arrayWithArray:_opaqueRenderItems];
    [renderList addObjectsFromArray:_transparentRenderItems];
    
    drawRenderList(renderList, renderEncoder);
    
    NSArray *copiedDeferredReusableBuffers = [_deferredReusableBuffers copy];
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
        dispatch_async(dispatch_get_main_queue(), ^{
            for (id<MTLBuffer> buffer in copiedDeferredReusableBuffers) {
                enqueueReusableBuffer(buffer);
            }
        });
    }];
    
    [_opaqueRenderItems removeAllObjects];
    [_transparentRenderItems removeAllObjects];
    [_currentLightNodes removeAllObjects];
    [_deferredReusableBuffers removeAllObjects];
}

void MetalRenderer::buildLightListRecursive(GLTFNode *node) {
    if (node.light != nil) {
        [_currentLightNodes addObject:node];
    }

    for (GLTFNode *childNode in node.children) {
        buildLightListRecursive(childNode);
    }
}

simd_float4x4 matrix_double_to_float(simd_double4x4 input) {
    simd_float4x4 tempResult;
    tempResult.columns[0][0] = input.columns[0][0];
    tempResult.columns[0][1] = input.columns[0][1];
    tempResult.columns[0][2] = input.columns[0][2];
    tempResult.columns[0][3] = input.columns[0][3];

    tempResult.columns[1][0] = input.columns[1][0];
    tempResult.columns[1][1] = input.columns[1][1];
    tempResult.columns[1][2] = input.columns[1][2];
    tempResult.columns[1][3] = input.columns[1][3];

    tempResult.columns[2][0] = input.columns[2][0];
    tempResult.columns[2][1] = input.columns[2][1];
    tempResult.columns[2][2] = input.columns[2][2];
    tempResult.columns[2][3] = input.columns[2][3];

    tempResult.columns[3][0] = input.columns[3][0];
    tempResult.columns[3][1] = input.columns[3][1];
    tempResult.columns[3][2] = input.columns[3][2];
    tempResult.columns[3][3] = input.columns[3][3];

    return tempResult;
}

simd_float3 double3_to_float3(simd_double3 input) {
    simd_float3 tempResult;
    tempResult.x = input.x;
    tempResult.y = input.y;
    tempResult.z = input.z;
    return tempResult;

}

void MetalRenderer::buildRenderListRecursive(std::shared_ptr<GLTFRenderModel> model,
                                             GLTFNode *node,
                                             simd_float4x4 modelMatrix,
                                             GLTFKHRLight *defaultAmbientLight) {
    modelMatrix = matrix_multiply(modelMatrix, node.localTransform);

    GLTFMesh *mesh = node.mesh;
    if (mesh) {
        for (GLTFSubmesh *submesh in mesh.submeshes) {
            GLTFMaterial *material = submesh.material;
            
            simd_double3x3 viewAffine = simd_inverse(GLTFMatrixUpperLeft3x3D(model->_modelViewMatrix));
            simd_double3 cameraPos = model->_modelViewMatrix.columns[3].xyz;
            simd_double3 cameraWorldPos = matrix_multiply(viewAffine, -cameraPos);
            simd_float3 cameraWorldPosF = double3_to_float3(cameraWorldPos);

            VertexUniforms vertexUniforms;
            vertexUniforms.modelMatrix = modelMatrix;

            // TODO: MT: Review this..
            auto mvp = matrix_multiply(_projectionMatrix, model->_modelMatrix);
            auto mvpF = matrix_double_to_float(mvp);
            vertexUniforms.modelViewProjectionMatrix = mvpF;
            auto normalMatrix = GLTFNormalMatrixFromModelMatrixD(model->_modelMatrix);
            vertexUniforms.normalMatrix = matrix_double_to_float(normalMatrix);

            vertexUniforms.scaleFactor = model->_scaling;
            vertexUniforms.brightness = model->_brightness;
            vertexUniforms.lightDirection = _renderingEnvironment->_lightDirection;

            FragmentUniforms fragmentUniforms;
            fragmentUniforms.normalScale = material.normalTextureScale;
            fragmentUniforms.emissiveFactor = material.emissiveFactor;
            fragmentUniforms.occlusionStrength = material.occlusionStrength;
            fragmentUniforms.metallicRoughnessValues = (simd_float2){ material.metalnessFactor, material.roughnessFactor };
            fragmentUniforms.baseColorFactor = material.baseColorFactor;
            fragmentUniforms.camera = cameraWorldPosF;
            fragmentUniforms.alphaCutoff = material.alphaCutoff;
            fragmentUniforms.envIntensity = 2; // self.lightingEnvironment.intensity;
            
            if (material.baseColorTexture != nil) {
                fragmentUniforms.textureMatrices[GLTFTextureBindIndexBaseColor] = GLTFTextureMatrixFromTransform(material.baseColorTexture.transform);
            }
            if (material.normalTexture != nil) {
                fragmentUniforms.textureMatrices[GLTFTextureBindIndexNormal] = GLTFTextureMatrixFromTransform(material.normalTexture.transform);
            }
            if (material.metallicRoughnessTexture != nil) {
                fragmentUniforms.textureMatrices[GLTFTextureBindIndexMetallicRoughness] = GLTFTextureMatrixFromTransform(material.metallicRoughnessTexture.transform);
            }
            if (material.occlusionTexture != nil) {
                fragmentUniforms.textureMatrices[GLTFTextureBindIndexOcclusion] = GLTFTextureMatrixFromTransform(material.occlusionTexture.transform);
            }
            if (material.emissiveTexture != nil) {
                fragmentUniforms.textureMatrices[GLTFTextureBindIndexEmissive] = GLTFTextureMatrixFromTransform(material.emissiveTexture.transform);
            }

            if (defaultAmbientLight != nil) {
                fragmentUniforms.ambientLight.color = defaultAmbientLight.color; // self.ambientLight.color;
                fragmentUniforms.ambientLight.intensity = defaultAmbientLight.intensity; //self.ambientLight.intensity;
            }

            // TODO: Make this more efficient. Iterating the light list for every submesh is pretty silly.
            for (unsigned int lightIndex = 0; lightIndex < _currentLightNodes.count; ++lightIndex) {
                GLTFNode *lightNode = _currentLightNodes[lightIndex];
                GLTFKHRLight *light = lightNode.light;
                if (light.type == GLTFKHRLightTypeDirectional) {
                    fragmentUniforms.lights[lightIndex].position = lightNode.globalTransform.columns[2];
                } else {
                    fragmentUniforms.lights[lightIndex].position = lightNode.globalTransform.columns[3];
                }
                fragmentUniforms.lights[lightIndex].color = light.color;
                fragmentUniforms.lights[lightIndex].intensity = light.intensity;
                fragmentUniforms.lights[lightIndex].range = light.range;
                if (light.type == GLTFKHRLightTypeSpot) {
                    fragmentUniforms.lights[lightIndex].innerConeAngle = light.innerConeAngle;
                    fragmentUniforms.lights[lightIndex].outerConeAngle = light.outerConeAngle;
                } else {
                    fragmentUniforms.lights[lightIndex].innerConeAngle = 0;
                    fragmentUniforms.lights[lightIndex].outerConeAngle = M_PI;
                }
                fragmentUniforms.lights[lightIndex].spotDirection = lightNode.globalTransform.columns[2];
            }
            
            GLTFMTLRenderItem *item = [GLTFMTLRenderItem new];
            item.label = [NSString stringWithFormat:@"%@ - %@", node.name ?: @"Unnamed node", submesh.name ?: @"Unnamed primitive"];
            item.node = node;
            item.submesh = submesh;
            item.vertexUniforms = vertexUniforms;
            item.fragmentUniforms = fragmentUniforms;
            
            if (submesh.material.alphaMode == GLTFAlphaModeBlend) {
                [_transparentRenderItems addObject:item];
            } else {
                [_opaqueRenderItems addObject:item];
            }
        }
    }
    
    for (GLTFNode *childNode in node.children) {
        buildRenderListRecursive(model,
                                 childNode, modelMatrix, defaultAmbientLight);
        // [self buildRenderListRecursive:childNode modelMatrix:modelMatrix];
    }
}

void MetalRenderer::drawRenderList(NSArray<GLTFMTLRenderItem *> *renderList,
                    id<MTLRenderCommandEncoder> renderEncoder) {
    for (GLTFMTLRenderItem *item in renderList) {
        GLTFNode *node = item.node;
        GLTFSubmesh *submesh = item.submesh;
        GLTFMaterial *material = submesh.material;
        
        [renderEncoder pushDebugGroup:[NSString stringWithFormat:@"%@", item.label]];
        
        id<MTLRenderPipelineState> renderPipelineState = renderPipelineStateForSubmesh(submesh);
                
        [renderEncoder setRenderPipelineState:renderPipelineState];
        
        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

        NSDictionary *accessorsForAttributes = submesh.accessorsForAttributes;
                
        GLTFAccessor *indexAccessor = submesh.indexAccessor;
        BOOL useIndexBuffer = (indexAccessor != nil);
                
        // TODO: Check primitive type for unsupported types (tri fan, line loop), and modify draw calls as appropriate
        MTLPrimitiveType primitiveType = GLTFMTLPrimitiveTypeForPrimitiveType(submesh.primitiveType);
        
        bindTexturesForMaterial(material, renderEncoder);
        
        VertexUniforms vertexUniforms = item.vertexUniforms;
        [renderEncoder setVertexBytes:&vertexUniforms length:sizeof(vertexUniforms) atIndex:GLTFVertexDescriptorMaxAttributeCount + 0];
        
        if (node.skin != nil && node.skin.jointNodes != nil && node.skin.jointNodes.count > 0) {
            id<MTLBuffer> jointBuffer = dequeueReusableBufferOfLength(node.skin.jointNodes.count * sizeof(simd_float4x4));
            computeJointsForSubmesh(submesh, node, jointBuffer);
            [renderEncoder setVertexBuffer:jointBuffer offset:0 atIndex:GLTFVertexDescriptorMaxAttributeCount + 1];
            [_deferredReusableBuffers addObject:jointBuffer];
        }
        
        FragmentUniforms fragmentUniforms = item.fragmentUniforms;
        [renderEncoder setFragmentBytes:&fragmentUniforms length: sizeof(fragmentUniforms) atIndex: 0];
                
        GLTFVertexDescriptor *vertexDescriptor = submesh.vertexDescriptor;
        for (int i = 0; i < GLTFVertexDescriptorMaxAttributeCount; ++i) {
            NSString *semantic = vertexDescriptor.attributes[i].semantic;
            if (semantic == nil) { continue; }
            GLTFAccessor *accessor = submesh.accessorsForAttributes[semantic];
            
            [renderEncoder setVertexBuffer:((GLTFMTLBuffer *)accessor.bufferView.buffer).buffer
                                    offset:accessor.offset + accessor.bufferView.offset
                                   atIndex:i];
        }
        
        if (material.alphaMode == GLTFAlphaModeBlend){
            id<MTLDepthStencilState> depthStencilState = depthStencilStateForDepthWriteEnabled(YES, YES, MTLCompareFunctionLess);
            [renderEncoder setDepthStencilState:depthStencilState];
        } else {
            id<MTLDepthStencilState> depthStencilState = depthStencilStateForDepthWriteEnabled(YES,YES,MTLCompareFunctionLess);
            [renderEncoder setDepthStencilState:depthStencilState];
        }
        
        if (material.isDoubleSided) {
            [renderEncoder setCullMode:MTLCullModeNone];
        } else {
            [renderEncoder setCullMode:MTLCullModeBack];
        }
        
        if (useIndexBuffer) {
            GLTFMTLBuffer *indexBuffer = (GLTFMTLBuffer *)indexAccessor.bufferView.buffer;
            
            MTLIndexType indexType = (indexAccessor.componentType == GLTFDataTypeUShort) ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
            
            [renderEncoder drawIndexedPrimitives:primitiveType
                                      indexCount:indexAccessor.count
                                       indexType:indexType
                                     indexBuffer:[indexBuffer buffer]
                               indexBufferOffset:indexAccessor.offset + indexAccessor.bufferView.offset];
        } else {
            GLTFAccessor *positionAccessor = accessorsForAttributes[GLTFAttributeSemanticPosition];
            [renderEncoder drawPrimitives:primitiveType vertexStart:0 vertexCount:positionAccessor.count];
        }
        
        [renderEncoder popDebugGroup];
    }
}


void MetalRenderer::bindTexturesForMaterial(GLTFMaterial *material,
                                            id<MTLRenderCommandEncoder> renderEncoder) {
    if (material.baseColorTexture != nil) {
        id<MTLTexture> texture = textureForImage(material.baseColorTexture.texture.image, true);
        id<MTLSamplerState> sampler = samplerStateForSampler(material.baseColorTexture.texture.sampler);
        [renderEncoder setFragmentTexture:texture atIndex:GLTFTextureBindIndexBaseColor];
        [renderEncoder setFragmentSamplerState:sampler atIndex:GLTFTextureBindIndexBaseColor];
    }
    
    if (material.normalTexture != nil) {
        id<MTLTexture> texture = textureForImage(material.normalTexture.texture.image, false);
        id<MTLSamplerState> sampler = samplerStateForSampler(material.normalTexture.texture.sampler);
        [renderEncoder setFragmentTexture:texture atIndex:GLTFTextureBindIndexNormal];
        [renderEncoder setFragmentSamplerState:sampler atIndex:GLTFTextureBindIndexNormal];
    }
    
    if (material.metallicRoughnessTexture != nil) {
        id<MTLTexture> texture = textureForImage(material.metallicRoughnessTexture.texture.image, false);
        id<MTLSamplerState> sampler = samplerStateForSampler(material.metallicRoughnessTexture.texture.sampler);
        [renderEncoder setFragmentTexture:texture atIndex:GLTFTextureBindIndexMetallicRoughness];
        [renderEncoder setFragmentSamplerState:sampler atIndex:GLTFTextureBindIndexMetallicRoughness];
    }
    
    if (material.emissiveTexture != nil) {
        id<MTLTexture> texture = textureForImage(material.emissiveTexture.texture.image, true);
        id<MTLSamplerState> sampler = samplerStateForSampler(material.emissiveTexture.texture.sampler);
        [renderEncoder setFragmentTexture:texture atIndex:GLTFTextureBindIndexEmissive];
        [renderEncoder setFragmentSamplerState:sampler atIndex:GLTFTextureBindIndexEmissive];
    }
    
    if (material.occlusionTexture != nil) {
        id<MTLTexture> texture = textureForImage(material.occlusionTexture.texture.image, false);
        id<MTLSamplerState> sampler = samplerStateForSampler(material.occlusionTexture.texture.sampler);
        [renderEncoder setFragmentTexture:texture atIndex:GLTFTextureBindIndexOcclusion];
        [renderEncoder setFragmentSamplerState:sampler atIndex:GLTFTextureBindIndexOcclusion];
    }
}

id<MTLBuffer> MetalRenderer::dequeueReusableBufferOfLength(size_t length) {
    int indexToReuse = -1;
    for (unsigned int i = 0; i < _bufferPool.count; ++i) {
        if (_bufferPool[i].length >= length) {
            indexToReuse = i;
        }
    }
    
    if (indexToReuse >= 0) {
        id <MTLBuffer> buffer = _bufferPool[indexToReuse];
        [_bufferPool removeObjectAtIndex:indexToReuse];
        return buffer;
    } else {
        return [_metalDevice newBufferWithLength:length options:MTLResourceStorageModeShared];
    }
}

void MetalRenderer::computeJointsForSubmesh(GLTFSubmesh *submesh,
                                            GLTFNode *node,
                                            id<MTLBuffer> jointBuffer) {
    GLTFAccessor *jointsAccessor = submesh.accessorsForAttributes[GLTFAttributeSemanticJoints0];
    GLTFSkin *skin = node.skin;
    GLTFAccessor *inverseBindingAccessor = node.skin.inverseBindMatricesAccessor;
    
    if (jointsAccessor != nil && inverseBindingAccessor != nil) {
        NSInteger jointCount = skin.jointNodes.count;
        simd_float4x4 *jointMatrices = (simd_float4x4 *)jointBuffer.contents;
        simd_float4x4 *inverseBindMatrices = (simd_float4x4 *)((char *)inverseBindingAccessor.bufferView.buffer.contents + inverseBindingAccessor.bufferView.offset + inverseBindingAccessor.offset);
        for (NSInteger i = 0; i < jointCount; ++i) {
            GLTFNode *joint = skin.jointNodes[i];
            simd_float4x4 inverseBindMatrix = inverseBindMatrices[i];
            jointMatrices[i] = matrix_multiply(matrix_invert(node.globalTransform), matrix_multiply(joint.globalTransform, inverseBindMatrix));
        }
    }
}

id<MTLDepthStencilState> MetalRenderer::depthStencilStateForDepthWriteEnabled(bool depthWriteEnabled,
                                                                              bool depthTestEnabled,
                                                                              MTLCompareFunction compareFunction) {
    
    NSInteger depthWriteBit = depthWriteEnabled ? 1 : 0;
    NSInteger depthTestBit = depthTestEnabled ? 1 : 0;
    
    NSInteger hash = (compareFunction << 2) | (depthWriteBit << 1) | depthTestBit;
    
    id <MTLDepthStencilState> depthStencilState = _depthStencilStateMap[@(hash)];
    if (depthStencilState) {
        return depthStencilState;
    }
    
    MTLDepthStencilDescriptor *depthDescriptor = [MTLDepthStencilDescriptor new];
    depthDescriptor.depthCompareFunction = depthTestEnabled ? compareFunction : MTLCompareFunctionAlways;
    depthDescriptor.depthWriteEnabled = depthWriteEnabled;
    depthStencilState = [_metalDevice newDepthStencilStateWithDescriptor:depthDescriptor];
    
    _depthStencilStateMap[@(hash)] = depthStencilState;
    
    return depthStencilState;
}


id<MTLTexture> MetalRenderer::textureForImage(GLTFImage *image,
                                              bool sRGB) {
//    NSParameterAssert(image != nil);
    
    id<MTLTexture> texture = _texturesForImageIdentifiers[image.identifier];
    
    if (texture) {
        return texture;
    }
    
    NSDictionary *options = @{ GLTFMTLTextureLoaderOptionGenerateMipmaps : @YES,
                               GLTFMTLTextureLoaderOptionSRGB : @(sRGB)
                             };
    
    NSError *error = nil;
    if (image.imageData != nil) {
        texture = [_textureLoader newTextureWithData:image.imageData options:options error:&error];
        texture.label = image.name;
    } else if (image.url != nil) {
        texture = [_textureLoader newTextureWithContentsOfURL:image.url options:options error:&error];
        texture.label = image.name ?: image.url.lastPathComponent;
    } else if (image.bufferView != nil) {
        GLTFBufferView *bufferView = image.bufferView;
        NSData *data = [NSData dataWithBytesNoCopy:(char *)bufferView.buffer.contents + bufferView.offset length:bufferView.length freeWhenDone:NO];
        texture = [_textureLoader newTextureWithData:data options:options error:&error];
        texture.label = image.name;
    }
    
    if (!texture) {
        NSLog(@"Error occurred while loading texture: %@", error);
    } else {
        _texturesForImageIdentifiers[image.identifier] = texture;
    }
    
    return texture;
}

id<MTLSamplerState> MetalRenderer::samplerStateForSampler(GLTFTextureSampler *sampler) {
//    NSParameterAssert(sampler != nil);
    
    id<MTLSamplerState> samplerState = _samplerStatesForSamplers[sampler];
    if (samplerState == nil) {
        MTLSamplerDescriptor *descriptor = [MTLSamplerDescriptor new];
        descriptor.magFilter = GLTFMTLSamplerMinMagFilterForSamplingFilter(sampler.magFilter);
        descriptor.minFilter = GLTFMTLSamplerMinMagFilterForSamplingFilter(sampler.minFilter);
        descriptor.mipFilter = GLTFMTLSamplerMipFilterForSamplingFilter(sampler.minFilter);
        descriptor.sAddressMode = GLTFMTLSamplerAddressModeForSamplerAddressMode(sampler.sAddressMode);
        descriptor.tAddressMode = GLTFMTLSamplerAddressModeForSamplerAddressMode(sampler.tAddressMode);
        descriptor.normalizedCoordinates = YES;
        samplerState = [_metalDevice newSamplerStateWithDescriptor:descriptor];
        _samplerStatesForSamplers[sampler] = samplerState;
    }
    return samplerState;
}

