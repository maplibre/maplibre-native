//
//  GLTFManager.hpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/25/24.
//

#ifndef GLTFManager_hpp
#define GLTFManager_hpp

#include <stdio.h>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "GLTFRenderer.hpp"
#include "GLTFModel.hpp"
#include "GLTFManagerRenderingEnvironment.hpp"

namespace maplibre {
namespace gltf {

// The type of rendering environment we're in
typedef enum {
    RenderingEnvironmentUnknown,
    RenderingEnvironmentMetal,
    RenderingEnvironmentOpenGL,
    RenderingEnvironmentVulkan
} RenderingEnvironment;

struct Coordinate2D {
    double _lat;
    double _lon;
};

struct Cartesian3D {
    double _x;
    double _y;
    double _z;
};

typedef std::function<Cartesian3D(const Coordinate2D &)> ProjectionCallback;

// The GLTF Manager class is the top level entry point
// for all things model based
class GLTFManager {
public:
    // Set the callback
    void setProjectionCallback(ProjectionCallback projectionCallback);

    // Instantiate the manager with the appropriate environment
    GLTFManager(RenderingEnvironment renderingEnvironment);

    // Load a model
    void addModel(std::shared_ptr<GLTFModel> model);

    // Remove a model
    void removeModel(std::shared_ptr<GLTFModel> model);

    // Current meter to screen pixel scale
    double _metersPerPixel = 1.0;

    // Set the drawable size
    void setDrawableSize(int width, int height);

    // Set the tilt deg
    void setTiltDeg(double tiltDeg);

    // Set the rotation deg
    void setRotationDeg(double rotationDeg);

    // Set the rendering environment variables
    void setRenderingEnvironmentVariables(std::shared_ptr<GLTFManagerRenderingEnvironment> environmentVars);

    // Update any animations
    void updateScene(float timeSinceLastDraw);

    // Render
    void render();

private:
    RenderingEnvironment _renderingEnvironment = RenderingEnvironmentUnknown;

    // List of models
    std::vector<std::shared_ptr<GLTFModel>> _models;

    // Projection callback
    ProjectionCallback _projectionCallback;

private:
    // Rendering environment
    void createRenderingEnvironment();

    // The renderer
    std::shared_ptr<GLTFRenderer> _renderer = nullptr;

    int _drawableWidth = 1;
    int _drawableHeight = 1;
    double _tiltDeg = 0;
    double _rotationDeg = 0;
};

} // namespace gltf
} // namespace maplibre

#endif /* GLTFManager_hpp */
