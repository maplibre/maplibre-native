#include <mbgl/plugin/plugin_layer_factory.hpp>
#include <mbgl/plugin/plugin_layer.hpp>
#include <mbgl/plugin/plugin_layer_impl.hpp>
#include <mbgl/plugin/plugin_layer_render.hpp>
#include <mbgl/plugin/feature_collection_bucket.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/renderer/bucket.hpp>

#include <string>

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
    _layerTypeInfo.type = _layerType.c_str();
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
}

void jsonStringFromConvertible(const style::conversion::Convertible& value, std::string& output) {
    if (isObject(value)) {
        output.append("{");
        bool firstItem = true;
        eachMember(value,
                   [&output, &firstItem](const std::string& name, const style::conversion::Convertible& paramValue)
                       -> std::optional<style::conversion::Error> {
                       if (!firstItem) {
                           output.append(",");
                       }
                       firstItem = false;
                       output.append("\"");
                       output.append(name);
                       output.append("\":");

                       jsonStringFromConvertible(paramValue, output);

                       return std::nullopt;
                   });
        output.append("}");
    } else if (isArray(value)) {
        output.append("[");
        bool firstItem = true;

        for (size_t i = 0; i < arrayLength(value); i++) {
            if (!firstItem) {
                output.append(",");
            }
            firstItem = false;

            auto itemValue = arrayMember(value, i);
            jsonStringFromConvertible(itemValue, output);
        }

        output.append("]");
    } else {
        auto v = toValue(value);
        if (auto iVal = v.value().getInt()) {
            std::string tempResult = std::to_string(*iVal);
            output.append(tempResult);
        } else if (auto uIVal = v.value().getUint()) {
            std::string tempResult = std::to_string(*uIVal);
            output.append(tempResult);

        } else if (auto s = v.value().getString()) {
            output.append("\"");
            output.append(*s);
            output.append("\"");

        } else if (auto d = v.value().getDouble()) {
            output.append(std::to_string(*d));
        }
    }
}

std::unique_ptr<style::Layer> PluginLayerFactory::createLayer(const std::string& id,
                                                              const style::conversion::Convertible& value) noexcept {
    std::string layerProperties;

    if (auto memberValue = objectMember(value, "properties")) {
        jsonStringFromConvertible(*memberValue, layerProperties);
        if (isObject(*memberValue)) {
            eachMember(*memberValue,
                       []([[maybe_unused]] const std::string& name,
                          [[maybe_unused]] const style::conversion::Convertible& paramValue)
                           -> std::optional<style::conversion::Error> { return std::nullopt; });
        }
    }

    std::string sourceStr = "pluginLayerNoSource";
    if (supportsFeatureCollectionBuckets) {
        auto source = getSource(value);
        if (source.has_value()) {
            sourceStr = source.value();
        }
    }

    auto tempResult = std::unique_ptr<style::Layer>(new (std::nothrow) style::PluginLayer(
        id, sourceStr, _layerTypeInfo, layerProperties
        //,*customProperties
        ));

    if (_onLayerCreated != nullptr) {
        auto layerRaw = tempResult.get();
        auto pluginLayer = static_cast<mbgl::style::PluginLayer*>(layerRaw);
        _onLayerCreated(pluginLayer);
        auto pluginLayerImpl = (mbgl::style::PluginLayer::Impl*)pluginLayer->baseImpl.get();
        if (pluginLayerImpl->_updateLayerPropertiesFunction != nullptr) {
            pluginLayerImpl->_updateLayerPropertiesFunction(layerProperties);
        }
    }

    return tempResult;
}

std::unique_ptr<Bucket> PluginLayerFactory::createBucket(
    [[maybe_unused]] const BucketParameters& parameters,
    [[maybe_unused]] const std::vector<Immutable<style::LayerProperties>>& layers) noexcept {
    if (supportsFeatureCollectionBuckets) {
        return std::make_unique<FeatureCollectionBucket>(parameters, layers);
    }
    return nullptr;
}

std::unique_ptr<RenderLayer> PluginLayerFactory::createRenderLayer(Immutable<style::Layer::Impl> impl) noexcept {
    auto localImpl = staticImmutableCast<style::PluginLayer::Impl>(impl);
    auto tempResult = std::make_unique<RenderPluginLayer>(staticImmutableCast<style::PluginLayer::Impl>(impl));
    tempResult->setRenderFunction(localImpl->_renderFunction);
    tempResult->setUpdateFunction(localImpl->_updateFunction);
    tempResult->setUpdatePropertiesFunction(localImpl->_updateLayerPropertiesFunction);
    return tempResult;
}

} // namespace mbgl
