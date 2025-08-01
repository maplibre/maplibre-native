#pragma once

#include <mbgl/layermanager/layer_factory.hpp>

#include <string>
#include <functional>

namespace mbgl {

namespace style {
// Forward
class PluginLayer;

} // namespace style

class PluginLayerFactory : public LayerFactory {
public:
    PluginLayerFactory(std::string& layerType,
                       mbgl::style::LayerTypeInfo::Source source,
                       mbgl::style::LayerTypeInfo::Pass3D pass3D,
                       mbgl::style::LayerTypeInfo::Layout layout,
                       mbgl::style::LayerTypeInfo::FadingTiles fadingTiles,
                       mbgl::style::LayerTypeInfo::CrossTileIndex crossTileIndex,
                       mbgl::style::LayerTypeInfo::TileKind tileKind);

    // Set to false, but if the caller wants to support on features loaded, then set this to true
    bool supportsFeatureCollectionBuckets = false;

    using OnLayerCreatedEvent = std::function<void(mbgl::style::PluginLayer* pluginLayer)>;
    void setOnLayerCreatedEvent(OnLayerCreatedEvent onLayerCreated) { _onLayerCreated = onLayerCreated; }

protected:
    const style::LayerTypeInfo* getTypeInfo() const noexcept final;
    std::unique_ptr<style::Layer> createLayer(const std::string& id,
                                              const style::conversion::Convertible& value) noexcept final;
    std::unique_ptr<Bucket> createBucket(const BucketParameters&,
                                         const std::vector<Immutable<style::LayerProperties>>&) noexcept final;
    std::unique_ptr<RenderLayer> createRenderLayer(Immutable<style::Layer::Impl>) noexcept final;

private:
    // These is the layer type info that is setup during factory creation and returned in the getTypeInfo method
    style::LayerTypeInfo _layerTypeInfo;
    std::string _layerType;

    OnLayerCreatedEvent _onLayerCreated;
};

} // namespace mbgl
