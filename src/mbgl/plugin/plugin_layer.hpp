#pragma once

#include <mbgl/style/layer.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/plugin/feature_collection.hpp>

namespace mbgl {

class RawBucketFeature;

namespace style {

class PluginLayer final : public Layer {
public:
    PluginLayer(const std::string& layerID,
                const std::string& sourceID,
                const style::LayerTypeInfo layerTypeInfo,
                const std::string& layerProperties);
    ~PluginLayer() override;

    // Private implementation
    class Impl;
    const Impl& impl() const;

    Mutable<Impl> mutableImpl() const;
    PluginLayer(Immutable<Impl>);
    std::unique_ptr<Layer> cloneRef(const std::string& id) const final;

public:
    using OnRenderLayer = std::function<void(PaintParameters&)>;
    using OnUpdateLayer = std::function<void(const LayerPrepareParameters&)>;
    using OnUpdateLayerProperties = std::function<void(const std::string& properties)>;
    using OnFeatureCollectionLoaded =
        std::function<void(const std::shared_ptr<plugin::FeatureCollection> featureCollection)>;
    using OnFeatureCollectionUnloaded =
        std::function<void(const std::shared_ptr<plugin::FeatureCollection> featureCollection)>;

    void* _platformReference = nullptr;

protected:
    std::optional<conversion::Error> setPropertyInternal(const std::string& name,
                                                         const conversion::Convertible& value) final;

    StyleProperty getProperty(const std::string& name) const final;
    Value serialize() const final;

    Mutable<Layer::Impl> mutableBaseImpl() const final;
};

} // namespace style
} // namespace mbgl
