//
//  plugin_layer_factory.cpp
//  MapLibre
//
//  Created by Malcolm Toon on 4/24/25.
//

#include "plugin_layer_factory.hpp"
#include "plugin_layer.hpp"
#include "plugin_layer_impl.hpp"
// #include "plugin_layer_bucket.hpp"

namespace mbgl {

namespace plugins {

// This is really hacky, but wanted to do it here to discuss if the
// const can be removed from teh regular LayerTypeInfo
struct NonConstLayerTypeInfo {
    const char* type;
    enum class Source {
        Required,
        NotRequired
    } source;
    enum class Pass3D {
        Required,
        NotRequired
    } pass3d;
    enum class Layout {
        Required,
        NotRequired
    } layout;
    enum class FadingTiles {
        Required,
        NotRequired
    } fadingTiles;
    enum class CrossTileIndex {
        Required,
        NotRequired
    } crossTileIndex;
    enum class TileKind : uint8_t {
        Geometry,
        Raster,
        RasterDEM,
        NotRequired
    } tileKind;
};

} // namespace plugins

style::LayerTypeInfo getDefaultInfo() {
    style::LayerTypeInfo tempResult = {.type = "unknown",
                                       .source = style::LayerTypeInfo::Source::Required,
                                       .pass3d = style::LayerTypeInfo::Pass3D::Required,
                                       .layout = style::LayerTypeInfo::Layout::NotRequired,
                                       .fadingTiles = style::LayerTypeInfo::FadingTiles::NotRequired,
                                       .crossTileIndex = style::LayerTypeInfo::CrossTileIndex::NotRequired,
                                       .tileKind = style::LayerTypeInfo::TileKind::Geometry};
    return tempResult;
}

PluginLayerFactory::PluginLayerFactory(std::string& layerType,
                                       mbgl::style::LayerTypeInfo::Source source,
                                       mbgl::style::LayerTypeInfo::Pass3D pass3D,
                                       mbgl::style::LayerTypeInfo::Layout layout,
                                       mbgl::style::LayerTypeInfo::FadingTiles fadingTiles,
                                       mbgl::style::LayerTypeInfo::CrossTileIndex crossTileIndex,
                                       mbgl::style::LayerTypeInfo::TileKind tileKind)
    : _layerTypeInfo(getDefaultInfo()),
      _layerType(layerType) {
    _layerTypeInfo.type = layerType.c_str();
    plugins::NonConstLayerTypeInfo* lti = (plugins::NonConstLayerTypeInfo*)&_layerTypeInfo;
    lti->source = (plugins::NonConstLayerTypeInfo::Source)((int)source);
    lti->pass3d = (plugins::NonConstLayerTypeInfo::Pass3D)((int)pass3D);
    lti->layout = (plugins::NonConstLayerTypeInfo::Layout)((int)layout);
    lti->fadingTiles = (plugins::NonConstLayerTypeInfo::FadingTiles)((int)fadingTiles);
    lti->crossTileIndex = (plugins::NonConstLayerTypeInfo::CrossTileIndex)((int)crossTileIndex);
    lti->tileKind = (plugins::NonConstLayerTypeInfo::TileKind)((int)tileKind);
}

const style::LayerTypeInfo* PluginLayerFactory::getTypeInfo() const noexcept {
    return &_layerTypeInfo;
    // return nullptr;
    // return style::PluginLayer::Impl::staticTypeInfo();
}

std::unique_ptr<style::Layer> PluginLayerFactory::createLayer(const std::string& id,
                                                              const style::conversion::Convertible& value) noexcept {
    // TODO: What is this and how does it fit in
    //    const auto source = getSource(value);
    //    if (!source) {
    //        return nullptr;
    //    }

    std::string source = "source";
    return std::unique_ptr<style::Layer>(new (std::nothrow) style::PluginLayer(id, source, _layerTypeInfo));

    /*
        const auto source = getSource(value);
        if (!source) {
            return nullptr;
        }
        return std::unique_ptr<style::Layer>(new (std::nothrow) style::PluginLayer(id, *source));
     */
}

std::unique_ptr<Bucket> PluginLayerFactory::createBucket(
    const BucketParameters& parameters, const std::vector<Immutable<style::LayerProperties>>& layers) noexcept {
    return nullptr;
    // return std::make_unique<PluginLayerBucket>(parameters, layers);
}

std::unique_ptr<RenderLayer> PluginLayerFactory::createRenderLayer(Immutable<style::Layer::Impl> impl) noexcept {
    return nullptr;

    //    assert(impl->getTypeInfo() == getTypeInfo());
    //    return std::make_unique<RenderHeatmapLayer>(staticImmutableCast<style::HeatmapLayer::Impl>(impl));
}

} // namespace mbgl
