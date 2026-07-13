#pragma once

#include <mbgl/style/layers/custom_drawable_layer.hpp>

class ExampleCustomDrawableStyleLayerHost : public mbgl::style::CustomDrawableLayerHost {
public:
    using VertexVector = mbgl::gfx::VertexVector<Interface::GeometryVertex>;
    using TriangleIndexVector = mbgl::gfx::IndexVector<mbgl::gfx::Triangles>;

    ExampleCustomDrawableStyleLayerHost(const std::string& assetsPath);
    ~ExampleCustomDrawableStyleLayerHost();

    void initialize() override;
    void deinitialize() override;

    void update(Interface& interface) override;

protected:
    static mbgl::Point<double> project(const mbgl::LatLng& c, const mbgl::TransformState& s);

    void createDrawables(Interface& interface);
    void generateGeometry(Interface& interface);
    void loadGeometry(Interface& interface);
    void importObj(Interface& interface,
                   const std::string& filename,
                   VertexVector& vertices,
                   TriangleIndexVector& indices,
                   Interface::GeometryOptions& options);

    mbgl::gfx::Texture2DPtr createCheckerboardTexture(Interface& interface,
                                                      uint16_t wb,
                                                      uint16_t hb,
                                                      uint16_t blockSize,
                                                      const std::array<uint8_t, 4>& color1,
                                                      const std::array<uint8_t, 4>& color2);

protected:
    const std::string assetsPath;
};
