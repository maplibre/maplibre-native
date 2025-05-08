#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/plugins/plugin_layer.hpp>
// #include <mbgl/style/layers/plugin_layer_properties.hpp>
#include <mbgl/style/conversion_impl.hpp>

namespace mbgl {
namespace style {

class PluginLayer::Impl : public Layer::Impl {
public:
    Impl(std::string layerID, std::string sourceID, LayerTypeInfo layerTypeInfo, const std::string& layerProperties
         //,const style::conversion::Convertible& layerProperties
    );

    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    const LayerTypeInfo* getTypeInfo() const noexcept final {
        return &_layerTypeInfo;
        // TODO: Return the right thing here
        // return nullptr;
    }

    void setRenderFunction(OnRenderLayer renderFunction) {
        _renderFunction = renderFunction;
    }
    
    void setUpdateFunction(OnUpdateLayer updateFunction) {
        _updateFunction = updateFunction;
    }


    OnRenderLayer _renderFunction;
    OnUpdateLayer _updateFunction;

private:
    LayerTypeInfo _layerTypeInfo;
    std::string _layerProperties;
    // style::conversion::Convertible _layerProperties;

    // HeatmapPaintProperties::Transitionable paint;

    // Not needed for this
    // DECLARE_LAYER_TYPE_INFO;

    //    const LayerTypeInfo* getTypeInfo() const noexcept final {
    //        return staticTypeInfo();
    //    }
    //    static const LayerTypeInfo* staticTypeInfo() noexcept;
};

} // namespace style
} // namespace mbgl
