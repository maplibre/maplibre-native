#pragma once

#include <mln/layermanager/layer_factory.hpp>

#include <string>
#include <functional>

namespace mln {

namespace style {
// Forward
class PluginLayer;

} // namespace style

class PluginLayerFactory : public LayerFactory {
public:
    PluginLayerFactory(std::string& layerType,
                       mln::style::LayerTypeInfo::Source source,
                       mln::style::LayerTypeInfo::Pass3D pass3D,
                       mln::style::LayerTypeInfo::Layout layout,
                       mln::style::LayerTypeInfo::FadingTiles fadingTiles,
                       mln::style::LayerTypeInfo::CrossTileIndex crossTileIndex,
                       mln::style::LayerTypeInfo::TileKind tileKind);

    using OnLayerCreatedEvent = std::function<void(mln::style::PluginLayer* pluginLayer)>;
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

} // namespace mln
