//
//  GLTFManagerRenderingEnvironment.hpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/25/24.
//

#ifndef GLTFManagerRenderingEnvironment_hpp
#define GLTFManagerRenderingEnvironment_hpp

#include <stdio.h>
#include <simd/simd.h>

namespace maplibre { namespace gltf {

    // Bsae class does nothing really, just a place holder
    class GLTFManagerRenderingEnvironment {
    public:
        // If the bloom should be used
        bool _useBloomPass = false;

        // FOV
        double _currentFOVDEG = 50;
        
        // Environment projection matrix
        simd_double4x4 _currentProjectionMatrix;
        
        // Current zoom level
        double _currentZoomLevel = 1;

    };

}}
#endif /* GLTFManagerRenderingEnvironment_hpp */
