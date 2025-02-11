//
//  GLTFModel.hpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/25/24.
//

#ifndef GLTFModel_hpp
#define GLTFModel_hpp

#include <stdio.h>
#include <string>

namespace maplibre {
namespace gltf {

// Model
class GLTFModel {
public:
    //
    double _referenceLat = 0;
    double _referenceLon = 0;

    // What it takes to scale this model to meters units
    float _scaleFactor = 1;

    // Rotation around the reference point
    float _rotationDeg = 0;

    // The URL for the model, used by the loader
    std::string _modelURL;

    // The brightness
    float _brightness = 1.0;

    // This is a hack to deal with passing in the translation vector.
    // NEed to reconcile this with the renderable model's version of it
    double _xLateX = 0;
    double _xLateY = 0;
    double _xLateZ = 0;
};

} // namespace gltf
} // namespace maplibre

#endif /* GLTFModel_hpp */
