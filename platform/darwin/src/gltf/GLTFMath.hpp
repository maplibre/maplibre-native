//
//  Math.hpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/19/24.
//

#ifndef Math_hpp
#define Math_hpp

#include <stdio.h>
#include <simd/simd.h>

typedef enum {
    GLTFTextureBindIndexBaseColor,
    GLTFTextureBindIndexNormal,
    GLTFTextureBindIndexMetallicRoughness,
    GLTFTextureBindIndexOcclusion,
    GLTFTextureBindIndexEmissive,
    GLTFTextureBindIndexDiffuseEnvironment,
    GLTFTextureBindIndexSpecularEnvironment,
    GLTFTextureBindIndexBRDFLookup,
} GLTFMTLTextureBindIndex;

#define DEG_RAD (M_PI / 180.0)
#define RAD_DEG (180.0 / M_PI)

//  0.0174533
// 57.2958

#define GLTFMTLMaximumLightCount 3
#define GLTFMTLMaximumTextureCount (GLTFTextureBindIndexEmissive + 1)

typedef struct {
    simd_float3 minPoint;
    simd_float3 maxPoint;
} GLTFBoundingBox;

typedef struct {
    simd_double3 center;
    float radius;
} GLTFBoundingSphere;

typedef struct {
    simd_float4x4 modelMatrix;
    simd_float4x4 modelViewProjectionMatrix;
    simd_float4x4 normalMatrix;
    float scaleFactor;
    float brightness;
    simd_float3 lightDirection;
} VertexUniforms;

typedef struct {
    simd_float4 position;
    simd_float4 color;
    float intensity;
    float innerConeAngle;
    float outerConeAngle;
    float range;
    simd_float4 spotDirection;
} Light;

typedef struct {
    float normalScale;
    simd_float3 emissiveFactor;
    float occlusionStrength;
    simd_float2 metallicRoughnessValues;
    simd_float4 baseColorFactor;
    simd_float3 camera;
    float alphaCutoff;
    float envIntensity;
    Light ambientLight;
    Light lights[GLTFMTLMaximumLightCount];
    simd_float3x3 textureMatrices[GLTFMTLMaximumTextureCount];
} FragmentUniforms;

typedef struct {
    simd_float2 offset;
    simd_float2 scale;
    float rotation;
} GLTFTextureTransform;

#ifdef __cplusplus
extern "C" {
#endif

extern simd_float3 GLTFAxisX;
extern simd_float3 GLTFAxisY;
extern simd_float3 GLTFAxisZ;

extern simd_double3 GLTFAxisXD;
extern simd_double3 GLTFAxisYD;
extern simd_double3 GLTFAxisZD;

simd_float4x4 GLTFMatrixFromScale(const simd_float3 s);

simd_double4x4 GLTFMatrixFromScaleD(const simd_double3 s);

simd_float4x4 GLTFMatrixFromUniformScale(float s);

simd_double4x4 GLTFMatrixFromUniformScaleD(double s);

GLTFBoundingSphere GLTFBoundingSphereFromBox(GLTFBoundingBox b);

simd_float4x4 GLTFMatrixFromTranslation(simd_float3 t);

simd_double4x4 GLTFMatrixFromTranslationD(simd_double3 t);

simd_float4x4 GLTFRotationMatrixFromAxisAngle(simd_float3 axis, float angle);

simd_double4x4 GLTFRotationMatrixFromAxisAngleD(simd_double3 axis, double angle);

simd_double4x4 GLTFPerspectiveProjectionMatrixAspectFovRH(const double fovY,
                                                          const double aspect,
                                                          const double nearZ,
                                                          const double farZ);

simd_float4x4 GLTFOrthoProjectionMatrix(
    const float left, const float right, const float bottom, const float top, const float nearZ, const float farZ);

simd_float3x3 GLTFMatrixUpperLeft3x3(simd_float4x4 m);

simd_double3x3 GLTFMatrixUpperLeft3x3D(simd_double4x4 m);

GLTFTextureTransform GLTFTextureTransformMakeIdentity(void);

GLTFTextureTransform GLTFTextureTransformMakeSRT(simd_float2 scale, float rotation, simd_float2 offset);

simd_float3x3 GLTFTextureMatrixFromTransform(GLTFTextureTransform transform);

simd_float4x4 GLTFNormalMatrixFromModelMatrix(simd_float4x4 m);

simd_double4x4 GLTFNormalMatrixFromModelMatrixD(simd_double4x4 m);

#ifdef __cplusplus
}
#endif

#endif /* Math_hpp */
