#include <mln/layermanager/layer_manager.hpp>

#include <mln/layout/layout.hpp>
#include <mln/layermanager/layer_factory.hpp>
#include <mln/renderer/bucket.hpp>
#include <mln/renderer/bucket_parameters.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/layer_impl.hpp>
#include <mln/style/conversion_impl.hpp>

#if MLN_WITH_PLUGINS
#include <map>
#include <mutex>
#include <set>
#endif

namespace mln {

#if MLN_WITH_PLUGINS
class LayerManager::Impl {
public:
    std::mutex runtimeMutex;
    std::map<std::string, std::unique_ptr<LayerFactory>> runtimeFactories;
};

LayerManager::LayerManager()
    : impl(std::make_unique<Impl>()) {}
#else
LayerManager::LayerManager() = default;
#endif

LayerManager::~LayerManager() = default;

#if MLN_WITH_PLUGINS
bool LayerManager::registerLayerFactories(std::vector<std::unique_ptr<LayerFactory>> factories, std::string& error) {
    std::lock_guard lock(impl->runtimeMutex);
    error.clear();
    std::set<std::string> names;
    for (const auto& factory : factories) {
        const auto* info = factory ? factory->getTypeInfo() : nullptr;
        if (!info || !info->type || !*info->type) {
            error = "Layer factory must have a non-empty, immutable type name";
            return false;
        }
        if (impl->runtimeFactories.contains(info->type) || !names.emplace(info->type).second) {
            error = "Layer type is already registered: " + std::string(info->type);
            return false;
        }
    }
    // Allocate all nodes before publishing any factory. In addition to conflict
    // validation, this keeps allocation failure from partially registering a batch.
    std::map<std::string, std::unique_ptr<LayerFactory>> pending;
    for (auto& factory : factories) {
        const std::string name = factory->getTypeInfo()->type;
        pending.emplace(name, std::move(factory));
    }
    impl->runtimeFactories.merge(pending);
    return true;
}

bool LayerManager::registerLayerFactory(std::unique_ptr<LayerFactory> factory, std::string& error) {
    std::vector<std::unique_ptr<LayerFactory>> factories;
    factories.push_back(std::move(factory));
    return registerLayerFactories(std::move(factories), error);
}

LayerFactory* LayerManager::findFactory(const std::string& type) noexcept {
    std::lock_guard lock(impl->runtimeMutex);
    const auto found = impl->runtimeFactories.find(type);
    return found == impl->runtimeFactories.end() ? getFactory(type) : found->second.get();
}

LayerFactory* LayerManager::findFactory(const style::LayerTypeInfo* info) noexcept {
    if (!info || !info->type) return nullptr;
    // An override only affects name-based creation. Existing built-in layers
    // must keep their renderer, even when their name now resolves to a plugin.
    std::lock_guard lock(impl->runtimeMutex);
    const auto found = impl->runtimeFactories.find(info->type);
    auto* factory = found == impl->runtimeFactories.end() ? nullptr : found->second.get();
    if (factory && factory->getTypeInfo() == info) return factory;
    // Preserve platform semantics for existing built-ins and legacy C++ types.
    return getFactory(info->type) ? getFactory(info) : nullptr;
}

bool LayerManager::hasLayerType(const std::string& type) noexcept {
    return findFactory(type) != nullptr;
}

#else
LayerFactory* LayerManager::findFactory(const std::string& type) noexcept {
    return getFactory(type);
}
LayerFactory* LayerManager::findFactory(const style::LayerTypeInfo* info) noexcept {
    return getFactory(info);
}
#endif

void LayerManager::addLayerTypeCoreOnly(std::unique_ptr<mln::LayerFactory>) {}

std::unique_ptr<style::Layer> LayerManager::createLayer(const std::string& type,
                                                        const std::string& id,
                                                        const style::conversion::Convertible& value,
                                                        style::conversion::Error& error) noexcept {
    LayerFactory* factory = findFactory(type);
    if (factory) {
        auto layer = factory->createLayer(id, value);
        if (!layer) {
            error.message = "Error parsing layer " + id + " of type: " + type;
        }
        return layer;
    } else {
        error.message = "Null factory for type: " + type;
    }
    error.message = "Unsupported layer type! " + error.message;
    return nullptr;
}

std::unique_ptr<Bucket> LayerManager::createBucket(const BucketParameters& parameters,
                                                   const std::vector<Immutable<style::LayerProperties>>& layers) {
    assert(!layers.empty());
    assert(parameters.layerType->layout == style::LayerTypeInfo::Layout::NotRequired);
    LayerFactory* factory = findFactory(parameters.layerType);
    assert(factory);
    return factory->createBucket(parameters, layers);
}

std::unique_ptr<Layout> LayerManager::createLayout(const LayoutParameters& parameters,
                                                   std::unique_ptr<GeometryTileLayer> tileLayer,
                                                   const std::vector<Immutable<style::LayerProperties>>& layers) {
    assert(!layers.empty());
    assert(parameters.bucketParameters.layerType->layout == style::LayerTypeInfo::Layout::Required);
    LayerFactory* factory = findFactory(parameters.bucketParameters.layerType);
    assert(factory);
    return factory->createLayout(parameters, std::move(tileLayer), layers);
}

std::unique_ptr<RenderLayer> LayerManager::createRenderLayer(Immutable<style::Layer::Impl> layerImpl) noexcept {
    LayerFactory* factory = findFactory(layerImpl->getTypeInfo());
    assert(factory);
    return factory->createRenderLayer(std::move(layerImpl));
}

} // namespace mln
