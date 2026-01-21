#pragma once

#include <stdio.h>
#include <string>
#include <mbgl/util/vectors.hpp>
#include <mbgl/util/mat4.hpp>
#include <vector>
#include <memory>

namespace mbgl {


class PaintParameters;

namespace plugin {

    // Forward
    class MapLayer;

    // This defines a single layer property
    class __attribute__((visibility("default"))) LayerProperty {
    public:
        
        // Types of properties
        enum class PropertyType {
            Unknown,
            SingleFloat,
            Color
        };
        
        // The property name
        std::string propertyName;
        
        // The type of property
        PropertyType propertyType;
        
        // Default float value
        float singleFloatDefaultValue = 0;
        
        // Default color value: RGBA
        bool hasDefaultColorValue = false;
        vec4 colorDefaultValue;
    };

    // This is the metadata about the map layer type that is used
    // to create the LayerTypeInfo
    class __attribute__((visibility("default"))) MapLayerType {
    public:
        static int layerTypeVersion() { return 1; }
        MapLayerType();
        virtual ~MapLayerType() __attribute__((used));

        // Returns the layer type string that is used in the style
        virtual std::string getLayerType() __attribute__((used));
        
        // If this layer type requires pass 3d
        virtual bool requiresPass3D() __attribute__((used));

        // This creates the actual map layer.  Should be overridden by the
        // implementor and return a class descended from the MapLayer below
        virtual std::shared_ptr<MapLayer> createMapLayer() __attribute__((used));
      
        // The list of properties
        virtual std::vector<std::shared_ptr<LayerProperty>> getLayerProperties() __attribute__((used));
           
    };


    // This is the drawing context that is passed into the update method
    class __attribute__((visibility("default"))) DrawingContext {
    public:
        vec2i drawableSize;
        vec2 centerCoordinate;
        float zoomLevel;
        float direction;
        float pitch;
        float fieldOfView;
        mat4 projectionMatrix;
        mat4 nearClippedProjMatrix;
    };

    // This is the rendering context of the map layer.  It's platform dependendant and will
    // be overridden by the 
    class __attribute__((visibility("default"))) RenderingContext {
    public:
        
    };

    // The map layer is a pure virtual class that can be overriden by the plugin system
    class __attribute__((visibility("default"))) MapLayer {
    public:

        virtual ~MapLayer();
        
        virtual void onRender(const RenderingContext *);
        
        virtual void onAddedToMap();
        
        virtual void onUpdate(const DrawingContext &);

        virtual void onUpdateLayerProperties(const std::string&);

        virtual void onMemoryReductionEvent();
        
    };

} }

// This will create a rendering context for the current platform and return it.
// Implemented by the platform
extern "C" mbgl::plugin::RenderingContext *createPlatformRenderingContext(mbgl::PaintParameters&);


// #endif /* PluginMapLayer_hpp */
