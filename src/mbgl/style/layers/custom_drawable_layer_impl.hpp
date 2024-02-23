#pragma once

#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/layer_properties.hpp>

#include <memory>

namespace mbgl {

class TransformState;

namespace style {

class CustomDrawableLayer::Impl : public Layer::Impl {
public:
    Impl(const std::string& id, std::unique_ptr<CustomDrawableLayerHost> host);

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    std::shared_ptr<CustomDrawableLayerHost> host;

    DECLARE_LAYER_TYPE_INFO;
};

class CustomDrawableLayerProperties final : public LayerProperties {
public:
    explicit CustomDrawableLayerProperties(Immutable<CustomDrawableLayer::Impl> impl)
        : LayerProperties(std::move(impl)) {}

    expression::Dependency getDependencies() const noexcept override { return expression::Dependency::None; }
};

} // namespace style
} // namespace mbgl
