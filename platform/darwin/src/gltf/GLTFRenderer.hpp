//
//  GLTFRenderer.hpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/19/24.
//

#ifndef GLTFRenderer_hpp
#define GLTFRenderer_hpp

#include <stdio.h>
#include "Camera.hpp"
#include "GLTFModel.hpp"
#include "GLTFManagerRenderingEnvironment.hpp"
#include <memory>

namespace maplibre {
namespace gltf {

class GLTFRenderer {
public:
    // Constructor
    GLTFRenderer();

    // Destructor
    virtual ~GLTFRenderer();

    // Update any animations
    virtual void update(float timeSinceLastDraw);

    // Render
    virtual void render();

    // Set the drawable size
    virtual void setDrawableSize(int width, int height);

    // Load a model
    virtual void loadGLTFModel(std::shared_ptr<GLTFModel> model);

    // Set the meters per pixel scale
    void setMetersPerPixel(double metersPerPixel);

    // Set tilt
    void setTiltDeg(double tiltDeg);

    // Set the rotation deg
    void setRotationDeg(double rotationDeg);

    // Use bloom pass
    void setUseBloomPass(bool useBloomPass);

    // Set the rendering environemnt variables
    virtual void setRenderingEnvironemnt(std::shared_ptr<GLTFManagerRenderingEnvironment> renderingEnvironment);

protected:
    std::unique_ptr<Camera> _camera = nullptr;
    double _metersPerPixel = 1;
    std::shared_ptr<GLTFManagerRenderingEnvironment> _renderingEnvironment = nullptr;
    double _tiltDeg = 0;
    double _rotationDeg = 0;
    bool _useBloomPass = true;
    virtual void loadBloomPipelines();
    virtual void loadTonemapPipeline();
    virtual void updateFramebufferSize();
};

} // namespace gltf
} // namespace maplibre

#endif /* GLTFRenderer_hpp */
