//
//  GLTFRenderer.cpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/19/24.
//

#include "GLTFRenderer.hpp"

using namespace maplibre::gltf;

GLTFRenderer::GLTFRenderer() {
    _camera = std::make_unique<Camera>();
}

// Destructor
GLTFRenderer::~GLTFRenderer() {
    // Base class does nothing
}

// Update any animations
void GLTFRenderer::update(float timeSinceLastDraw) {
    // Base class does nothing
}

// Render
void GLTFRenderer::render() {
    // Base class does nothing
}

// Set the drawable size
void GLTFRenderer::setDrawableSize(int width, int height) {
    // Base class does nothing
}

// Protected items
void GLTFRenderer::loadBloomPipelines() {}

void GLTFRenderer::loadTonemapPipeline() {}

void GLTFRenderer::updateFramebufferSize() {}

// Load a model
void GLTFRenderer::loadGLTFModel(std::shared_ptr<GLTFModel> model) {
    // Base class does nothing
}

// Set the meters per pixel scale
void GLTFRenderer::setMetersPerPixel(double metersPerPixel) {
    _metersPerPixel = metersPerPixel;
}

// Set tilt
void GLTFRenderer::setTiltDeg(double tiltDeg) {
    _tiltDeg = tiltDeg;
}

// Set the rotation deg
void GLTFRenderer::setRotationDeg(double rotationDeg) {
    _rotationDeg = rotationDeg;
}

// Use bloom pass
void GLTFRenderer::setUseBloomPass(bool useBloomPass) {
    _useBloomPass = useBloomPass;
}

// Set the rendering environemnt variables
void GLTFRenderer::setRenderingEnvironemnt(std::shared_ptr<GLTFManagerRenderingEnvironment> renderingEnvironment) {
    _renderingEnvironment = renderingEnvironment;
}
