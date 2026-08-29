#include <mbgl/layermanager/layer_manager.hpp>

#include <mbgl/layout/layout.hpp>
#include <mbgl/layout/plugin_layout.hpp>
#include <mbgl/layermanager/layer_factory.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/renderer/layers/render_plugin_style_layer.hpp>
#include <mbgl/plugin/plugin_registry.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/layers/plugin_style_layer.hpp>
#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/conversion_impl.hpp>

namespace mbgl {

void LayerManager::addLayerTypeCoreOnly(std::unique_ptr<mbgl::LayerFactory>) {}

std::unique_ptr<style::Layer> LayerManager::createLayer(const std::string& type,
                                                        const std::string& id,
                                                        const style::conversion::Convertible& value,
                                                        style::conversion::Error& error) noexcept {
    LayerFactory* factory = getFactory(type);
    if (factory) {
        auto layer = factory->createLayer(id, value);
        if (!layer) {
            error.message = "Error parsing layer " + id + " of type: " + type;
        }
        return layer;
    } else {
        if (auto registration = plugin::PluginRegistry::get().findLayerType(type)) {
            std::string source;
            if (registration->sourceKind == MLN_PLUGIN_SOURCE_GEOMETRY) {
                const auto sourceValue = objectMember(value, "source");
                const auto sourceID = sourceValue ? toString(*sourceValue) : std::nullopt;
                if (!sourceID || sourceID->empty()) {
                    error.message = "Plugin layer '" + id + "' of type '" + type + "' requires a source";
                    return nullptr;
                }
                source = *sourceID;
            }
            return std::make_unique<style::PluginStyleLayer>(id, source, std::move(*registration));
        }
        error.message = "Null factory for type: " + type;
    }
    error.message = "Unsupported layer type! " + error.message;
    return nullptr;
}

std::unique_ptr<Bucket> LayerManager::createBucket(const BucketParameters& parameters,
                                                   const std::vector<Immutable<style::LayerProperties>>& layers) {
    assert(!layers.empty());
    assert(parameters.layerType->layout == style::LayerTypeInfo::Layout::NotRequired);
    LayerFactory* factory = getFactory(parameters.layerType);
    assert(factory);
    return factory->createBucket(parameters, layers);
}

std::unique_ptr<Layout> LayerManager::createLayout(const LayoutParameters& parameters,
                                                   std::unique_ptr<GeometryTileLayer> tileLayer,
                                                   const std::vector<Immutable<style::LayerProperties>>& layers) {
    assert(!layers.empty());
    assert(parameters.bucketParameters.layerType->layout == style::LayerTypeInfo::Layout::Required);
    LayerFactory* factory = getFactory(parameters.bucketParameters.layerType);
    if (!factory && layers.front()->baseImpl->isPluginStyleLayer()) {
        const auto& impl = static_cast<const style::PluginStyleLayer::Impl&>(*layers.front()->baseImpl);
        return std::make_unique<PluginLayout>(
            parameters.bucketParameters, layers, std::move(tileLayer), impl.registration);
    }
    assert(factory);
    return factory->createLayout(parameters, std::move(tileLayer), layers);
}

std::unique_ptr<RenderLayer> LayerManager::createRenderLayer(Immutable<style::Layer::Impl> impl) noexcept {
    if (impl->isPluginStyleLayer()) {
        return std::make_unique<RenderPluginStyleLayer>(
            staticImmutableCast<style::PluginStyleLayer::Impl>(std::move(impl)));
    }
    LayerFactory* factory = getFactory(impl->getTypeInfo());
    if (factory) return factory->createRenderLayer(std::move(impl));
    assert(false && "No layer factory for registered style layer");
    return nullptr;
}

} // namespace mbgl
