#pragma once

/*
 Plug-in layer scope

 v1:
    * At the platform level, be able to register a plug-in layer that is then parseable by the style parser
    * The plug-in layer will be limited to simple rendering (via handing off the rendering
      context to the plug-in layer) and will not include the ability to define drawables/etc
    * The "paint" properties will be parseable, support expressions and passed in frame by frame
    * A custom set of "plugin-propeties" will also be available at the same level as the "paint" properties
    * The plug-in layer will be notified about lifecycle events (creation, addition to the mapview, removal,
      destruction/etc) and be expected to manage it's own resources

 v2:
    * TBD
 */

#include <mbgl/style/layer.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_layer.hpp>

namespace mbgl {

namespace style {

class PluginLayer final : public Layer {
public:
    PluginLayer(const std::string& layerID,
                const std::string& sourceID,
                const style::LayerTypeInfo layerTypeInfo,
                const std::string& layerProperties

                //,const style::conversion::Convertible& layerProperties
    );
    ~PluginLayer() override;

    // Private implementation
    class Impl;
    const Impl& impl() const;

    Mutable<Impl> mutableImpl() const;
    PluginLayer(Immutable<Impl>);
    std::unique_ptr<Layer> cloneRef(const std::string& id) const final;

public:
    typedef std::function<void(PaintParameters&)> OnRenderLayer;
    typedef std::function<void(const LayerPrepareParameters&)> OnUpdateLayer;
    typedef std::function<void(const std::string& properties)> OnUpdateLayerProperties;

protected:
    std::optional<conversion::Error> setPropertyInternal(const std::string& name,
                                                         const conversion::Convertible& value) final;

    StyleProperty getProperty(const std::string& name) const final;
    Value serialize() const final;

    Mutable<Layer::Impl> mutableBaseImpl() const final;
};

} // namespace style
} // namespace mbgl
