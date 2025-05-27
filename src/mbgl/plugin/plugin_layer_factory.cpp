#include "plugin_layer_factory.hpp"
#include "plugin_layer.hpp"
#include "plugin_layer_impl.hpp"
#include "plugin_layer_render.hpp"
// #include "plugin_layer_bucket.hpp"
#include <mbgl/style/conversion_impl.hpp>
#include <iostream>
#include <string>
#include "plugin_layer_debug.hpp"

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
}

void jsonStringFromConvertible(const style::conversion::Convertible& value, std::string& output) {
    if (isObject(value)) {
        output.append("{");
        bool firstItem = true;
        eachMember(value,
                   [&output, &firstItem](const std::string& name, const style::conversion::Convertible& value)
                       -> std::optional<style::conversion::Error> {
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
                       std::cout << "Working on: " << name << "\n";
#endif

                       if (!firstItem) {
                           output.append(",");
                       }
                       firstItem = false;
                       output.append("\"");
                       output.append(name);
                       output.append("\":");

                       jsonStringFromConvertible(value, output);

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
        /*
         DECLARE_VALUE_TYPE_ACCESOR(Int, int64_t)
         DECLARE_VALUE_TYPE_ACCESOR(Uint, uint64_t)
         DECLARE_VALUE_TYPE_ACCESOR(Bool, bool)
         DECLARE_VALUE_TYPE_ACCESOR(Double, double)
         DECLARE_VALUE_TYPE_ACCESOR(Array, array_type)
         DECLARE_VALUE_TYPE_ACCESOR(Object, object_type)
         DECLARE_VALUE_TYPE_ACCESOR(String, std::string)

         */
        auto v = toValue(value);
        if (auto i = v.value().getInt()) {
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
            std::cout << "Int: " << i << "\n";
#endif
            std::string tempResult = std::to_string(*i);
            output.append(tempResult);
        } else if (auto i = v.value().getUint()) {
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
            std::cout << "Int: " << i << "\n";
#endif
            std::string tempResult = std::to_string(*i);
            output.append(tempResult);

        } else if (auto s = v.value().getString()) {
            output.append("\"");
            output.append(v.value().getString()->c_str());
            output.append("\"");

            //  std::cout << "String: " << v.value().getString()->c_str() << "\n";
        } else if (auto d = v.value().getDouble()) {
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
            std::cout << "Double: " << d << "\n";
#endif

            output.append(std::to_string(*d));
        }
        //        std::cout << v.value().getInt() << "\n";
        //        if (v == std::nullopt) {
        //            // This means there's no value.. skip
        //        } else if (v.has_value()) {
        //
        //        }
    }
}

std::unique_ptr<style::Layer> PluginLayerFactory::createLayer(const std::string& id,
                                                              const style::conversion::Convertible& value) noexcept {
    // TODO: What is this and how does it fit in
    //    const auto source = getSource(value);
    //    if (!source) {
    //        return nullptr;
    //    }

    std::string layerProperties;

    if (auto memberValue = objectMember(value, "properties")) {
        jsonStringFromConvertible(*memberValue, layerProperties);
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
        std::cout << "Properties: " << layerProperties << "\n";
#endif
        if (isObject(*memberValue)) {
            eachMember(*memberValue,
                       [](const std::string& name, const style::conversion::Convertible& value)
                           -> std::optional<style::conversion::Error> { return std::nullopt; });
        }

        // if (isArray(memberValue)) {
        //            for (size_t i=0; i < arrayLength(memberValue); i++) {
        //                auto itemValue = arrayMember(memberValue, i);
        //            }
        //}
        //        if (auto error_ = layer->setProperty(member, *memberValue)) {
        //            error = *error_;
        //            return false;
        //        }
    }

    std::string source = "source";

    auto tempResult = std::unique_ptr<style::Layer>(new (std::nothrow)
                                                        style::PluginLayer(id, source, _layerTypeInfo, layerProperties
                                                                           //,*customProperties
                                                                           ));

#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    std::cout << "tempResultCreated: " << id << "\n";
#endif

    if (_onLayerCreated != nullptr) {
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
        std::cout << "_onLayerCreated: " << id << "\n";
#endif

        auto layerRaw = tempResult.get();
        auto pluginLayer = static_cast<mbgl::style::PluginLayer*>(layerRaw);
        _onLayerCreated(pluginLayer);
        auto pluginLayerImpl = (mbgl::style::PluginLayer::Impl*)pluginLayer->baseImpl.get();
        if (pluginLayerImpl->_updateLayerPropertiesFunction != nullptr) {
            pluginLayerImpl->_updateLayerPropertiesFunction(layerProperties);
        }
    }

    return tempResult;
    /*
    return std::unique_ptr<style::Layer>(new (std::nothrow) style::PluginLayer(id
                                                                               , source
                                                                               , _layerTypeInfo
                                                                               , layerProperties
                                                                               //,*customProperties
                                                                               ));
*/
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
    // Returning null for now.  Not using buckets in plug-ins yet.
    return nullptr;
}

std::unique_ptr<RenderLayer> PluginLayerFactory::createRenderLayer(Immutable<style::Layer::Impl> impl) noexcept {
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    std::cout << "Create Render Layer\n";
#endif

    auto localImpl = staticImmutableCast<style::PluginLayer::Impl>(impl);
    auto tempResult = std::make_unique<RenderPluginLayer>(staticImmutableCast<style::PluginLayer::Impl>(impl));
    tempResult->setRenderFunction(localImpl->_renderFunction);
    tempResult->setUpdateFunction(localImpl->_updateFunction);
    tempResult->setUpdatePropertiesFunction(localImpl->_updateLayerPropertiesFunction);
    return tempResult;
}

} // namespace mbgl
