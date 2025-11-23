#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/layers/color_relief_layer.hpp>
#include <mbgl/style/layers/color_relief_layer_properties.hpp>

namespace mbgl {
namespace style {

class ColorReliefLayer::Impl : public Layer::Impl {
public:
    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    static const LayerTypeInfo* staticTypeInfo() noexcept;

    ColorReliefPaintProperties::Transitionable paint;
};

} // namespace style
} // namespace mbgl