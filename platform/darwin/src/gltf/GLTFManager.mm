//
//  GLTFManager.cpp
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/25/24.
//

#include "GLTFManager.hpp"
#include "MetalRenderer.hpp"
#include "GLTFManagerRenderingEnvironmentMetal.hpp"
#include <memory>

using namespace maplibre::gltf;

// Set the callback
void GLTFManager::setProjectionCallback(ProjectionCallback projectionCallback) {
    _projectionCallback = projectionCallback;
}

// Instantiate the manager with the appropriate environment
GLTFManager::GLTFManager(RenderingEnvironment renderingEnvironment) {
    _renderingEnvironment = renderingEnvironment;
    createRenderingEnvironment();
}

// Load a model
void GLTFManager::addModel(std::shared_ptr<GLTFModel> model) {
    
    _models.push_back(model);
    if (_renderer != nullptr) {
        _renderer->loadGLTFModel(model);
    }
    
    
    
}

// Remove a model
void GLTFManager::removeModel(std::shared_ptr<GLTFModel> model) {
    
    _models.erase(std::find(_models.begin(), _models.end(), model));
        
}

// Set the drawable size
void GLTFManager::setDrawableSize(int width, int height) {
    
    _drawableWidth = width;
    _drawableHeight = height;
    
    if (_renderer != nullptr) {
        _renderer->setDrawableSize(_drawableWidth, _drawableHeight);
    }
    
}


// Set the tilt deg
void GLTFManager::setTiltDeg(double tiltDeg) {
    
    _tiltDeg = tiltDeg;
    if (_renderer != nullptr) {
        _renderer->setTiltDeg(_tiltDeg);
    }
    
}


// Set the rotation deg
void GLTFManager::setRotationDeg(double rotationDeg) {
    
    _rotationDeg = rotationDeg;
    if (_renderer != nullptr) {
        _renderer->setRotationDeg(_rotationDeg);
    }
}



// Set the rendering environment variables
void GLTFManager::setRenderingEnvironmentVariables(std::shared_ptr<GLTFManagerRenderingEnvironment> environmentVars) {
    
    if (_renderingEnvironment == RenderingEnvironmentMetal) {
        
        if (_renderer != nullptr) {
            
            // Set metal specific stuff
            auto mr = std::static_pointer_cast<MetalRenderer>(_renderer);
            auto mevars = std::static_pointer_cast<GLTFManagerRenderingEnvironmentMetal>(environmentVars);
            mr->setMetalDevice(mevars->_metalDevice);
            //mr->setCurrentDrawable(mevars->_currentDrawable);
            //mr->setCurrentOutputRenderPassDescriptor(mevars->_currentRenderPassDescriptor);
            
            // TODO: Add depth buffer variable management
            
            // TODO: Add bloom pass toggle
            mr->setUseBloomPass(mevars->_useBloomPass);

            // TODO: Remove all the above and use this instead
            mr->setRenderingEnvironemnt(environmentVars);
            
            
        }
            
    }
    
}


// Update any animations
void GLTFManager::updateScene(float timeSinceLastDraw) {
    if (_renderer == nullptr) {
        return;
    }
    
    // Go through all the models and project them.  This is a
    // hack right now to try and sync the models with the scene
    if (_projectionCallback != nullptr) {
        
        for (auto m: _models) {
            Coordinate2D c;
            c._lat = m->_referenceLat;
            c._lon = m->_referenceLon;
            
            
            
            auto cartesian = _projectionCallback(c);
            m->_xLateX = cartesian._x;
            m->_xLateY = cartesian._y;
            m->_xLateZ = cartesian._z;
            
        }
        
    }
    
    
    _renderer->setMetersPerPixel(_metersPerPixel);
    _renderer->update(timeSinceLastDraw);
}

// Render
void GLTFManager::render() {
    if (_renderer == nullptr) {
        return;
    }
    _renderer->render();
}



void GLTFManager::createRenderingEnvironment() {
    
    if (_renderingEnvironment == RenderingEnvironmentMetal) {

        // Create the renderer
        _renderer = std::make_shared<MetalRenderer>();
        
        // Set the drawable size
        _renderer->setDrawableSize(_drawableWidth, _drawableHeight);
        
        // Set the initial tilt deg
        _renderer->setTiltDeg(_tiltDeg);

        // Set metal specific stuff
        auto mr = std::static_pointer_cast<MetalRenderer>(_renderer);
        
        
    }
    
}
