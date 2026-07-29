#pragma once

#ifdef __cplusplus

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace mbgl {

class PaintParameters;

namespace plugin {

class MapLayer;

class __attribute__((visibility("default"))) LayerProperty {
public:
    enum class PropertyType {
        Unknown,
        SingleFloat,
        Color
    };

    std::string propertyName;
    PropertyType propertyType = PropertyType::Unknown;
    float singleFloatDefaultValue = 0;
    bool hasDefaultColorValue = false;
    std::array<double, 4> colorDefaultValue{};
};

class __attribute__((visibility("default"))) MapLayerType {
public:
    static int layerTypeVersion() { return 1; }
    MapLayerType();
    virtual ~MapLayerType() __attribute__((used));

    virtual std::string getLayerType() __attribute__((used));
    virtual bool requiresPass3D() __attribute__((used));
    virtual std::shared_ptr<MapLayer> createMapLayer() __attribute__((used));
    virtual std::vector<std::shared_ptr<LayerProperty>> getLayerProperties() __attribute__((used));
};

class __attribute__((visibility("default"))) DrawingContext {
public:
    std::array<int, 2> drawableSize{};
    std::array<double, 2> centerCoordinate{};
    float zoomLevel = 0;
    float direction = 0;
    float pitch = 0;
    float fieldOfView = 0;
    std::array<double, 16> projectionMatrix{};
    std::array<double, 16> nearClippedProjMatrix{};
};

class __attribute__((visibility("default"))) RenderingContext {
public:
};

class __attribute__((visibility("default"))) MapLayer {
public:
    virtual ~MapLayer();

    virtual void onRender(const RenderingContext*);
    virtual void onAddedToMap();
    virtual void onUpdate(const DrawingContext&);
    virtual void onUpdateLayerProperties(const std::string&);
    virtual void onMemoryReductionEvent();
};

} // namespace plugin
} // namespace mbgl

extern "C" mbgl::plugin::RenderingContext* createPlatformRenderingContext(mbgl::PaintParameters&);

#endif
