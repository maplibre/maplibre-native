//
//  Camera.hpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/19/24.
//

#ifndef Camera_hpp
#define Camera_hpp

#include <stdio.h>
#include <simd/simd.h>

namespace maplibre {
namespace gltf {

class Camera {
public:
    simd_float3 _rotationAngles;
    simd_float3 _rotationVelocity;
    double _velocity;
    float _distance;
    Camera();
    simd_double4x4 projectionMatrix();
    simd_double4x4 _viewMatrix;
    void updateWithTimestep(double timestep);
};

} // namespace gltf
} // namespace maplibre

#endif /* Camera_hpp */
