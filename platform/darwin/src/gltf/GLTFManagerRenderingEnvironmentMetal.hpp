//
//  GLTFManagerRenderingEnvironmentMetal.hpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/25/24.
//

#ifndef GLTFManagerRenderingEnvironmentMetal_hpp
#define GLTFManagerRenderingEnvironmentMetal_hpp

#include <stdio.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>
#include "GLTFManagerRenderingEnvironment.hpp"

namespace maplibre {
namespace gltf {

// Bsae class does nothing really, just a place holder
class GLTFManagerRenderingEnvironmentMetal : public GLTFManagerRenderingEnvironment {
public:
    // Allows command buffer and render encoder to be passed in.
    // if these are nil, then the renderer will create them
    id<MTLRenderCommandEncoder> _currentCommandEncoder = nullptr;
    id<MTLCommandBuffer> _currentCommandBuffer = nullptr;

    id<CAMetalDrawable> _currentDrawable = nullptr;
    MTLRenderPassDescriptor *_currentRenderPassDescriptor = nullptr;
    id<MTLDevice> _metalDevice = nullptr;

    // TBD: These are placeholders as we noodle how to integrate with ML
    // Depth descriptor: If this is null, then use an internal depth
    id<MTLTexture> _depthStencilTexture = nullptr;

    id<MTLTexture> _colorTexture = nullptr;
};

} // namespace gltf
} // namespace maplibre

#endif /* GLTFManagerRenderingEnvironmentMetal_hpp */
