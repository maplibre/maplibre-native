#pragma once

#include <mln/style/layers/custom_drawable_layer.hpp>

class ExampleCustomDrawableStyleLayerHost : public mln::style::CustomDrawableLayerHost {
public:
    using VertexVector = mln::gfx::VertexVector<Interface::GeometryVertex>;
    using TriangleIndexVector = mln::gfx::IndexVector<mln::gfx::Triangles>;

    ExampleCustomDrawableStyleLayerHost(const std::string& assetsPath);
    ~ExampleCustomDrawableStyleLayerHost();

    void initialize() override;
    void deinitialize() override;

    void update(Interface& interface) override;

protected:
    static mln::Point<double> project(const mln::LatLng& c, const mln::TransformState& s);

    void createDrawables(Interface& interface);
    void generateGeometry(Interface& interface);
    void loadGeometry(Interface& interface);
    void importObj(Interface& interface,
                   const std::string& filename,
                   VertexVector& vertices,
                   TriangleIndexVector& indices,
                   Interface::GeometryOptions& options);

    mln::gfx::Texture2DPtr createCheckerboardTexture(Interface& interface,
                                                     uint16_t wb,
                                                     uint16_t hb,
                                                     uint16_t blockSize,
                                                     const std::array<uint8_t, 4>& color1,
                                                     const std::array<uint8_t, 4>& color2);

protected:
    const std::string assetsPath;
};
