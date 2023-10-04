#pragma once

#include <mbgl/style/layer.hpp>

#include <array>

namespace mbgl {
namespace style {

class CustomDrawableLayerHost {
public: 
    struct PaintParameters {
        double width;
        double height;
        double latitude;
        double longitude;
        double zoom;
        double bearing;
        double pitch;
        double fieldOfView;
        std::array<double, 16> projectionMatrix;
    };

public:
    virtual ~CustomDrawableLayerHost() = default;

    virtual void initialize() = 0;

    virtual void upload() = 0;

    virtual void draw(const PaintParameters&) = 0;

    virtual void update() = 0;

    virtual void deinitialize() = 0;
};

class CustomDrawableLayer final : public Layer {
public:
    CustomDrawableLayer(const std::string& id, std::unique_ptr<CustomDrawableLayerHost> host);

    CustomDrawableLayer(const CustomDrawableLayer&) = delete;
    ~CustomDrawableLayer() final;
    class Impl;
    const Impl& impl() const;
    Mutable<Impl> mutableImpl() const;

private:
    std::optional<conversion::Error> setPropertyInternal(const std::string& name,
                                                         const conversion::Convertible& value) final;
    StyleProperty getProperty(const std::string&) const final;
    std::unique_ptr<Layer> cloneRef(const std::string& id) const final;
    Mutable<Layer::Impl> mutableBaseImpl() const final;
};

} // namespace style
} // namespace mbgl
