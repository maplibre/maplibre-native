//
//  Camera.cpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/19/24.
//

#include "Camera.hpp"
#include "GLTFMath.hpp"

const float GLTFViewerOrbitCameraDefaultDistance = 2;
const float GLTFViewerOrbitCameraZoomDrag = 0.95;
const float GLTFViewerOrbitCameraRotationDrag = 0.6667;
const float GLTFViewerOrbitCameraZoomSensitivity = 2;

using namespace maplibre::gltf;

Camera::Camera() {
    _rotationAngles = 0;
    _rotationVelocity = 0;
    _velocity = 0;
    _distance = 4; // 0; // 2;
}

simd_double4x4 Camera::projectionMatrix() {
    float fov = M_PI / 3;     // original
    fov = 0.6435011087932844; // Constant that i found in ML
    simd_double4x4 matrix = GLTFPerspectiveProjectionMatrixAspectFovRH(fov, 1.0, 0.01, 250);
    return matrix;
}

void Camera::updateWithTimestep(double timestep) {
    _rotationAngles += _rotationVelocity * timestep;

    static double rot = 0;
    // rot += 0.03;
    //    self.rotationAngles = rot;
    //    _rotationAngles = simd_make_float3(rot, M_PI/4.0, 0);
    _rotationAngles = simd_make_float3(rot, 0, 0);

    // Clamp pitch
    _rotationAngles = (simd_float3){
        _rotationAngles.x, static_cast<float>(fmax(-M_PI_2, fmin(_rotationAngles.y, M_PI_2))), 0};

    _rotationVelocity *= GLTFViewerOrbitCameraRotationDrag;

    // _distance += _velocity * timestep;
    _velocity *= GLTFViewerOrbitCameraZoomDrag;

    simd_double4x4 pitchRotation = GLTFRotationMatrixFromAxisAngleD(GLTFAxisXD, -_rotationAngles.y);
    simd_double4x4 yawRotation = GLTFRotationMatrixFromAxisAngleD(GLTFAxisYD, -_rotationAngles.x);
    simd_double4x4 translation = GLTFMatrixFromTranslationD((simd_double3){0, 0, _distance});
    _viewMatrix = matrix_invert(matrix_multiply(matrix_multiply(yawRotation, pitchRotation), translation));
}
