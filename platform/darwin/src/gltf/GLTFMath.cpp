//
//  Math.cpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/19/24.
//

#include "GLTFMath.hpp"

simd_float4x4 GLTFMatrixFromScale(const simd_float3 s) {
    simd_float4x4 m = matrix_identity_float4x4;
    m.columns[0].x = s.x;
    m.columns[1].y = s.y;
    m.columns[2].z = s.z;
    return m;
}

simd_double4x4 GLTFMatrixFromScaleD(const simd_double3 s) {
    simd_double4x4 m = matrix_identity_double4x4;
    m.columns[0].x = s.x;
    m.columns[1].y = s.y;
    m.columns[2].z = s.z;
    return m;
}

simd_float4x4 GLTFMatrixFromUniformScale(float s) {
    simd_float4x4 m = matrix_identity_float4x4;
    m.columns[0].x = s;
    m.columns[1].y = s;
    m.columns[2].z = s;
    return m;
}

simd_double4x4 GLTFMatrixFromUniformScaleD(double s) {
    simd_double4x4 m = matrix_identity_double4x4;
    m.columns[0].x = s;
    m.columns[1].y = s;
    m.columns[2].z = s;
    return m;
}

simd_double4x4 GLTFMatrixFromTranslationD(simd_double3 t) {
    simd_double4x4 m = matrix_identity_double4x4;
    m.columns[3] = (simd_double4){t.x, t.y, t.z, 1.0};
    return m;
}

simd_float4x4 GLTFMatrixFromTranslation(simd_float3 t) {
    simd_float4x4 m = matrix_identity_float4x4;
    m.columns[3] = (simd_float4){t.x, t.y, t.z, 1.0};
    return m;
}

GLTFBoundingSphere GLTFBoundingSphereFromBox(const GLTFBoundingBox b) {
    GLTFBoundingSphere s;
    double midX = (b.maxPoint.x + b.minPoint.x) * 0.5;
    double midY = (b.maxPoint.y + b.minPoint.y) * 0.5;
    double midZ = (b.maxPoint.z + b.minPoint.z) * 0.5;

    double r = sqrt(pow(b.maxPoint.x - midX, 2) + pow(b.maxPoint.y - midY, 2) + pow(b.maxPoint.z - midZ, 2));

    s.center = (simd_double3){midX, midY, midZ};
    s.radius = r;
    return s;
}

simd_float4x4 GLTFRotationMatrixFromAxisAngle(simd_float3 axis, float angle) {
    float x = axis.x, y = axis.y, z = axis.z;
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1 - c;

    simd_float4 c0 = {t * x * x + c, t * x * y + z * s, t * x * z - y * s, 0};
    simd_float4 c1 = {t * x * y - z * s, t * y * y + c, t * y * z + x * s, 0};
    simd_float4 c2 = {t * x * z + y * s, t * y * z - x * s, t * z * z + c, 0};
    simd_float4 c3 = {0, 0, 0, 1};

    return (simd_float4x4){c0, c1, c2, c3};
}

simd_double4x4 GLTFRotationMatrixFromAxisAngleD(simd_double3 axis, double angle) {
    double x = axis.x, y = axis.y, z = axis.z;
    double c = cos(angle);
    double s = sin(angle);
    double t = 1 - c;

    simd_double4 c0 = {t * x * x + c, t * x * y + z * s, t * x * z - y * s, 0};
    simd_double4 c1 = {t * x * y - z * s, t * y * y + c, t * y * z + x * s, 0};
    simd_double4 c2 = {t * x * z + y * s, t * y * z - x * s, t * z * z + c, 0};
    simd_double4 c3 = {0, 0, 0, 1};

    return (simd_double4x4){c0, c1, c2, c3};
}

simd_float3 GLTFAxisX = (simd_float3){1, 0, 0};
simd_float3 GLTFAxisY = (simd_float3){0, 1, 0};
simd_float3 GLTFAxisZ = (simd_float3){0, 0, 1};

simd_double3 GLTFAxisXD = (simd_double3){1, 0, 0};
simd_double3 GLTFAxisYD = (simd_double3){0, 1, 0};
simd_double3 GLTFAxisZD = (simd_double3){0, 0, 1};

simd_double4x4 GLTFPerspectiveProjectionMatrixAspectFovRH(const double fovY,
                                                          const double aspect,
                                                          const double nearZ,
                                                          const double farZ) {
    double yscale = 1 / tanf(fovY * 0.5f); // 1 / tan == cot
    double xscale = yscale / aspect;
    double q = -farZ / (farZ - nearZ);

    simd_double4x4 m = {.columns[0] = {xscale, 0, 0, 0},
                        .columns[1] = {0, yscale, 0, 0},
                        .columns[2] = {0, 0, q, -1},
                        .columns[3] = {0, 0, q * nearZ, 0}};

    return m;
}

simd_float4x4 GLTFOrthoProjectionMatrix(
    const float left, const float right, const float bottom, const float top, const float nearZ, const float farZ) {
    simd_float4x4 m = {
        .columns[0] = {2 / (right - left), 0, 0, 0},
        .columns[1] = {0, 2 / (top - bottom), 0, 0},
        .columns[2] = {0, 0, 1 / (farZ - nearZ), 0},
        .columns[3] = {(left + right) / (left - right), (top + bottom) / (bottom - top), nearZ / (nearZ - farZ), 1}};

    return m;
}

simd_float3x3 GLTFMatrixUpperLeft3x3(simd_float4x4 m) {
    simd_float3x3 mout = {{{m.columns[0][0], m.columns[0][1], m.columns[0][2]},
                           {m.columns[1][0], m.columns[1][1], m.columns[1][2]},
                           {m.columns[2][0], m.columns[2][1], m.columns[2][2]}}};
    return mout;
}

simd_double3x3 GLTFMatrixUpperLeft3x3D(simd_double4x4 m) {
    simd_double3x3 mout = {{{m.columns[0][0], m.columns[0][1], m.columns[0][2]},
                            {m.columns[1][0], m.columns[1][1], m.columns[1][2]},
                            {m.columns[2][0], m.columns[2][1], m.columns[2][2]}}};
    return mout;
}

GLTFTextureTransform GLTFTextureTransformMakeIdentity(void) {
    GLTFTextureTransform t = {
        .offset = (simd_float2){0, 0},
        .scale = (simd_float2){1, 1},
        .rotation = 0,
    };
    return t;
}

GLTFTextureTransform GLTFTextureTransformMakeSRT(simd_float2 scale, float rotation, simd_float2 offset) {
    GLTFTextureTransform t = {
        .offset = offset,
        .scale = scale,
        .rotation = rotation,
    };
    return t;
}

simd_float3x3 GLTFTextureMatrixFromTransform(GLTFTextureTransform transform) {
    float rs = sinf(-transform.rotation);
    float rc = cosf(-transform.rotation);
    float tx = transform.offset.x;
    float ty = transform.offset.y;
    float sx = transform.scale.x;
    float sy = transform.scale.y;
    simd_float3 c0 = (simd_float3){rc * sx, rs * sy, 0};
    simd_float3 c1 = (simd_float3){-rs * sx, rc * sy, 0};
    simd_float3 c2 = (simd_float3){tx, ty, 1};
    simd_float3x3 m = (simd_float3x3){c0, c1, c2};
    return m;
}

simd_float4x4 GLTFNormalMatrixFromModelMatrix(simd_float4x4 m) {
    simd_float3x3 nm = simd_inverse(simd_transpose(GLTFMatrixUpperLeft3x3(m)));
    simd_float4x4 mout = {{{nm.columns[0][0], nm.columns[0][1], nm.columns[0][2], 0},
                           {nm.columns[1][0], nm.columns[1][1], nm.columns[1][2], 0},
                           {nm.columns[2][0], nm.columns[2][1], nm.columns[2][2], 0},
                           {0, 0, 0, 1}}};
    return mout;
}

simd_double4x4 GLTFNormalMatrixFromModelMatrixD(simd_double4x4 m) {
    simd_double3x3 nm = simd_inverse(simd_transpose(GLTFMatrixUpperLeft3x3D(m)));
    simd_double4x4 mout = {{{nm.columns[0][0], nm.columns[0][1], nm.columns[0][2], 0},
                            {nm.columns[1][0], nm.columns[1][1], nm.columns[1][2], 0},
                            {nm.columns[2][0], nm.columns[2][1], nm.columns[2][2], 0},
                            {0, 0, 0, 1}}};
    return mout;
}
