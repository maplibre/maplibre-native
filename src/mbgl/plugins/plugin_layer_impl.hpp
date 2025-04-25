#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/plugins/plugin_layer.hpp>
// #include <mbgl/style/layers/plugin_layer_properties.hpp>

namespace mbgl {
namespace style {

class PluginLayer::Impl : public Layer::Impl {
public:
    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    // HeatmapPaintProperties::Transitionable paint;

    DECLARE_LAYER_TYPE_INFO;
};

} // namespace style
} // namespace mbgl
