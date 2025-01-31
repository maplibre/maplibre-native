#pragma once

#include <mbgl/style/layers/custom_drawable_layer.hpp>

class ExampleCustomDrawableStyleLayerHost : public mbgl::style::CustomDrawableLayerHost {
public:
    ExampleCustomDrawableStyleLayerHost();

    void initialize() override;
    void deinitialize() override;

    void update(Interface &interface);

protected:

    void createDrawables(Interface& interface);
    void generateCommonGeometry(Interface& interface);
    void loadCommonGeometry(Interface& interface);

    void updateDrawables(Interface& interface);

protected:

    float commonGeometryBearing = 0.0f;
    mbgl::util::SimpleIdentity commonGeometryID;
};