#pragma once

#include <string>
#include <mbgl/layermanager/layer_factory.hpp>

namespace mbgl {

class PluginLayerFactory : public LayerFactory {
public:
    PluginLayerFactory(std::string& layerType);

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
};

} // namespace mbgl
