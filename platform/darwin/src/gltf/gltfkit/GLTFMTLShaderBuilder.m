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


#import "GLTFMTLShaderBuilder.h"
#import "GLTFMesh.h"
#import "GLTFMaterial.h"
#import "GLTFVertexDescriptor.h"
#import "GLTFAccessor.h"
#import "../GLTFMath.hpp"

@implementation GLTFMTLShaderBuilder

- (id<MTLRenderPipelineState>)renderPipelineStateForSubmesh:(GLTFSubmesh *)submesh
                                      //  lightingEnvironment:(GLTFMTLLightingEnvironment *)lightingEnvironment
                                           colorPixelFormat:(MTLPixelFormat)colorPixelFormat
                                    depthStencilPixelFormat:(MTLPixelFormat)depthStencilPixelFormat
                                                sampleCount:(int)sampleCount
                                                     device:(id<MTLDevice>)device
{
    NSParameterAssert(submesh);
    NSParameterAssert(submesh.material);
    NSParameterAssert(submesh.vertexDescriptor);
    
    NSError *error = nil;
    NSString *shaderSource = [self shaderSource];
    
    shaderSource = [self rewriteSource:shaderSource forSubmesh:submesh];
    
    id<MTLLibrary> library = [device newLibraryWithSource:shaderSource options:nil error:&error];
    if (!library) {
        NSLog(@"Error occurred while creating library for material : %@", error);
        return nil;
    }

    id <MTLFunction> vertexFunction = nil;
    id <MTLFunction> fragmentFunction = nil;

    for (NSString *functionName in [library functionNames]) {
        id<MTLFunction> function = [library newFunctionWithName:functionName];
        if ([function functionType] == MTLFunctionTypeVertex) {
            vertexFunction = function;
        } else if ([function functionType] == MTLFunctionTypeFragment) {
            fragmentFunction = function;
        }
    }
    
    if (!vertexFunction || !fragmentFunction) {
        NSLog(@"Failed to find a vertex and fragment function in library source");
        return nil;
    }
    
    MTLVertexDescriptor *vertexDescriptor = [self vertexDescriptorForSubmesh: submesh];

    MTLRenderPipelineDescriptor *pipelineDescriptor = [MTLRenderPipelineDescriptor new];
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    
    pipelineDescriptor.colorAttachments[0].pixelFormat = colorPixelFormat;
    pipelineDescriptor.sampleCount = sampleCount;

    if (submesh.material.alphaMode == GLTFAlphaModeBlend) {
        pipelineDescriptor.colorAttachments[0].blendingEnabled = YES;
        pipelineDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        pipelineDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        pipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
        pipelineDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
        pipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pipelineDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    }

    pipelineDescriptor.depthAttachmentPixelFormat = depthStencilPixelFormat;
    pipelineDescriptor.stencilAttachmentPixelFormat = depthStencilPixelFormat;
    
    id<MTLRenderPipelineState> pipeline = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
    if (!pipeline) {
        NSLog(@"Error occurred when creating render pipeline state: %@", error);
    }
    
    return pipeline;
}

- (NSString *)shaderSource {
    NSError *error = nil;
    // TODO: Figure out why Bazel won't embed the .metal files
    NSURL *shaderURL = [[NSBundle mainBundle] URLForResource:@"pbr" withExtension:@"txt"];
    if (shaderURL == nil) {
        NSLog(@"ERROR: Shader source not found in main bundle; pipeline states cannot be generated");
    }
    return [NSString stringWithContentsOfURL:shaderURL encoding:NSUTF8StringEncoding error:&error];
}

- (NSString *)rewriteSource:(NSString *)source
                 forSubmesh:(GLTFSubmesh *)submesh
{
    GLTFMaterial *material = submesh.material;
    
    BOOL usePBR = YES;
    BOOL useIBL = NO; // lightingEnvironment != nil;
    BOOL useDoubleSided = material.isDoubleSided;
    BOOL hasTexCoord0 = submesh.accessorsForAttributes[GLTFAttributeSemanticTexCoord0] != nil;
    BOOL hasTexCoord1 = submesh.accessorsForAttributes[GLTFAttributeSemanticTexCoord1] != nil;
    BOOL hasNormals = submesh.accessorsForAttributes[GLTFAttributeSemanticNormal] != nil;
    BOOL hasTangents = submesh.accessorsForAttributes[GLTFAttributeSemanticTangent] != nil;
    BOOL hasBaseColorMap = material.baseColorTexture != nil;
    BOOL hasOcclusionMap = material.occlusionTexture != nil;
    BOOL hasEmissiveMap = material.emissiveTexture != nil;
    BOOL hasNormalMap = material.normalTexture != nil;
    BOOL hasMetallicRoughnessMap = material.metallicRoughnessTexture != nil;
    BOOL hasTextureTransforms = material.hasTextureTransforms;
    BOOL hasSkinningData = submesh.accessorsForAttributes[GLTFAttributeSemanticJoints0] != nil &&
                           submesh.accessorsForAttributes[GLTFAttributeSemanticWeights0] != nil;
    BOOL hasExtendedSkinning = submesh.accessorsForAttributes[GLTFAttributeSemanticJoints1] != nil &&
                               submesh.accessorsForAttributes[GLTFAttributeSemanticWeights1] != nil;
    BOOL hasVertexColor = submesh.accessorsForAttributes[GLTFAttributeSemanticColor0] != nil;
    BOOL vertexColorIsRGB = submesh.accessorsForAttributes[GLTFAttributeSemanticColor0].dimension == GLTFDataDimensionVector3;
    BOOL hasVertexRoughness = submesh.accessorsForAttributes[GLTFAttributeSemanticRoughness] != nil;
    BOOL hasVertexMetallic = submesh.accessorsForAttributes[GLTFAttributeSemanticMetallic] != nil;
    BOOL premultiplyBaseColor = material.alphaMode == GLTFAlphaModeBlend;
    BOOL materialIsUnlit = material.isUnlit;
    BOOL useAlphaTest = material.alphaMode == GLTFAlphaModeMask;

    NSMutableString *shaderFeatures = [NSMutableString string];
    [shaderFeatures appendFormat:@"#define USE_PBR %d\n", usePBR];
    [shaderFeatures appendFormat:@"#define USE_IBL %d\n", useIBL];
    [shaderFeatures appendFormat:@"#define USE_ALPHA_TEST %d\n", useAlphaTest];
    [shaderFeatures appendFormat:@"#define USE_VERTEX_SKINNING %d\n", hasSkinningData];
    [shaderFeatures appendFormat:@"#define USE_EXTENDED_VERTEX_SKINNING %d\n", hasExtendedSkinning];
    [shaderFeatures appendFormat:@"#define USE_DOUBLE_SIDED_MATERIAL %d\n", useDoubleSided];
    [shaderFeatures appendFormat:@"#define HAS_TEXCOORD_0 %d\n", hasTexCoord0];
    [shaderFeatures appendFormat:@"#define HAS_TEXCOORD_1 %d\n", hasTexCoord1];
    [shaderFeatures appendFormat:@"#define HAS_NORMALS %d\n", hasNormals];
    [shaderFeatures appendFormat:@"#define HAS_TANGENTS %d\n", hasTangents];
    [shaderFeatures appendFormat:@"#define HAS_VERTEX_COLOR %d\n", hasVertexColor];
    [shaderFeatures appendFormat:@"#define VERTEX_COLOR_IS_RGB %d\n", vertexColorIsRGB];
    [shaderFeatures appendFormat:@"#define HAS_BASE_COLOR_MAP %d\n", hasBaseColorMap];
    [shaderFeatures appendFormat:@"#define HAS_NORMAL_MAP %d\n", hasNormalMap];
    [shaderFeatures appendFormat:@"#define HAS_METALLIC_ROUGHNESS_MAP %d\n", hasMetallicRoughnessMap];
    [shaderFeatures appendFormat:@"#define HAS_OCCLUSION_MAP %d\n", hasOcclusionMap];
    [shaderFeatures appendFormat:@"#define HAS_EMISSIVE_MAP %d\n", hasEmissiveMap];
    [shaderFeatures appendFormat:@"#define HAS_VERTEX_ROUGHNESS %d\n", hasVertexRoughness];
    [shaderFeatures appendFormat:@"#define HAS_VERTEX_METALLIC %d\n", hasVertexMetallic];
    [shaderFeatures appendFormat:@"#define HAS_TEXTURE_TRANSFORM %d\n", hasTextureTransforms];
    [shaderFeatures appendFormat:@"#define PREMULTIPLY_BASE_COLOR %d\n", premultiplyBaseColor];
    [shaderFeatures appendFormat:@"#define MATERIAL_IS_UNLIT %d\n", materialIsUnlit];
    [shaderFeatures appendFormat:@"#define SPECULAR_ENV_MIP_LEVELS %d\n", 0]; // lightingEnvironment.specularMipLevelCount];
    [shaderFeatures appendFormat:@"#define MAX_LIGHTS %d\n", (int)GLTFMTLMaximumLightCount];
    [shaderFeatures appendFormat:@"#define MAX_MATERIAL_TEXTURES %d\n\n", (int)GLTFMTLMaximumTextureCount];

    [shaderFeatures appendFormat:@"#define BaseColorTexCoord          texCoord%d\n", (int)material.baseColorTexture.texCoord];
    [shaderFeatures appendFormat:@"#define NormalTexCoord             texCoord%d\n", (int)material.normalTexture.texCoord];
    [shaderFeatures appendFormat:@"#define MetallicRoughnessTexCoord  texCoord%d\n", (int)material.metallicRoughnessTexture.texCoord];
    [shaderFeatures appendFormat:@"#define EmissiveTexCoord           texCoord%d\n", (int)material.emissiveTexture.texCoord];
    [shaderFeatures appendFormat:@"#define OcclusionTexCoord          texCoord%d\n\n", (int)material.occlusionTexture.texCoord];

    NSString *preamble = @"struct VertexIn {\n";
    NSString *epilogue = @"\n};";
    
    NSMutableArray *attribs = [NSMutableArray array];
    int i = 0;
    for (GLTFVertexAttribute *attribute in submesh.vertexDescriptor.attributes) {
        if (attribute.componentType == GLTFBaseTypeUnknown) { continue; }
        if ([attribute.semantic isEqualToString:GLTFAttributeSemanticPosition]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ position  [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticNormal]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ normal    [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticTangent]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ tangent   [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticTexCoord0]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ texCoord0 [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticTexCoord1]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ texCoord1 [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticColor0]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ color     [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticJoints0]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ joints0  [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticJoints1]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ joints1  [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticWeights0]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ weights0  [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticWeights1]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ weights1  [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticRoughness]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ roughness [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        } else if ([attribute.semantic isEqualToString:GLTFAttributeSemanticMetallic]) {
            [attribs addObject:[NSString stringWithFormat:@"    %@ metalness [[attribute(%d)]];", GLTFMTLTypeNameForType(attribute.componentType, attribute.dimension, false), i]];
        }
        
        ++i;
    }
    
    NSString *decls = [NSString stringWithFormat:@"%@%@%@%@",
                       shaderFeatures, preamble, [attribs componentsJoinedByString:@"\n"], epilogue];
    
    NSRange startSigilRange = [source rangeOfString:@"/*%begin_replace_decls%*/"];
    NSRange endSigilRange = [source rangeOfString:@"/*%end_replace_decls%*/"];
    
    NSRange declRange = NSUnionRange(startSigilRange, endSigilRange);
    
    source = [source stringByReplacingCharactersInRange:declRange withString:decls];

    return source;
}

- (MTLVertexDescriptor *)vertexDescriptorForSubmesh:(GLTFSubmesh *)submesh {
    MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor new];
    
    GLTFVertexDescriptor *descriptor = submesh.vertexDescriptor;
    
    for (NSInteger attributeIndex = 0; attributeIndex < GLTFVertexDescriptorMaxAttributeCount; ++attributeIndex) {
        GLTFVertexAttribute *attribute = descriptor.attributes[attributeIndex];
        GLTFBufferLayout *layout = descriptor.bufferLayouts[attributeIndex];
        
        if (attribute.componentType == 0) {
            continue;
        }
        
        MTLVertexFormat vertexFormat = GLTFMTLVertexFormatForComponentTypeAndDimension(attribute.componentType, attribute.dimension);
        
        vertexDescriptor.attributes[attributeIndex].offset = 0;
        vertexDescriptor.attributes[attributeIndex].format = vertexFormat;
        vertexDescriptor.attributes[attributeIndex].bufferIndex = attributeIndex;
        
        vertexDescriptor.layouts[attributeIndex].stride = layout.stride;
        vertexDescriptor.layouts[attributeIndex].stepRate = 1;
        vertexDescriptor.layouts[attributeIndex].stepFunction = MTLVertexStepFunctionPerVertex; // MTLStepFunctionPerVertex;
    }

    return vertexDescriptor;
}

@end
