#include <mbgl/layermanager/layer_manager.hpp>

#include <mbgl/layermanager/background_layer_factory.hpp>
#include <mbgl/layermanager/circle_layer_factory.hpp>
#include <mbgl/layermanager/color_relief_layer_factory.hpp>
#include <mbgl/layermanager/custom_layer_factory.hpp>
#include <mbgl/layermanager/fill_extrusion_layer_factory.hpp>
#include <mbgl/layermanager/fill_layer_factory.hpp>
#include <mbgl/layermanager/heatmap_layer_factory.hpp>
#include <mbgl/layermanager/hillshade_layer_factory.hpp>
#include <mbgl/layermanager/line_layer_factory.hpp>
#include <mbgl/layermanager/location_indicator_layer_factory.hpp>
#include <mbgl/layermanager/raster_layer_factory.hpp>
#include <mbgl/layermanager/symbol_layer_factory.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/layermanager/custom_drawable_layer_factory.hpp>

#include <map>
#include <memory>
#include <vector>

namespace mbgl {

class LayerManagerDefault final : public LayerManager {
public:
    LayerManagerDefault();

    /**
     * Enables a layer type for JSON style only.
     *
     * We might not want to expose runtime API for some layer types
     * in order to save binary size (the corresponding SDK layer wrappers
     * should be excluded from the project build).
     */
    void addLayerTypeCoreOnly(std::unique_ptr<mbgl::LayerFactory>) override;

private:
    void addLayerType(std::unique_ptr<LayerFactory>);
    // LayerManager overrides.
    LayerFactory* getFactory(const std::string& type) noexcept final;
    LayerFactory* getFactory(const style::LayerTypeInfo*) noexcept final;

    std::vector<std::unique_ptr<LayerFactory>> factories;
    std::map<std::string, LayerFactory*> typeToFactory;
};

void LayerManagerDefault::addLayerTypeCoreOnly(std::unique_ptr<mbgl::LayerFactory> layerFactory) {
    addLayerType(std::move(layerFactory));
}

LayerManagerDefault::LayerManagerDefault() {
#if !defined(MBGL_LAYER_FILL_DISABLE_ALL)
    addLayerType(std::make_unique<FillLayerFactory>());
#endif
#if !defined(MBGL_LAYER_LINE_DISABLE_ALL)
    addLayerType(std::make_unique<LineLayerFactory>());
#endif
#if !defined(MBGL_LAYER_CIRCLE_DISABLE_ALL)
    addLayerType(std::make_unique<CircleLayerFactory>());
#endif
#if !defined(MBGL_LAYER_SYMBOL_DISABLE_ALL)
    addLayerType(std::make_unique<SymbolLayerFactory>());
#endif
#if !defined(MBGL_LAYER_RASTER_DISABLE_ALL)
    addLayerType(std::make_unique<RasterLayerFactory>());
#endif
#if !defined(MBGL_LAYER_BACKGROUND_DISABLE_ALL)
    addLayerType(std::make_unique<BackgroundLayerFactory>());
#endif
#if !defined(MBGL_LAYER_HILLSHADE_DISABLE_ALL)
    addLayerType(std::make_unique<HillshadeLayerFactory>());
#endif
#if !defined(MBGL_LAYER_COLOR_RELIEF_DISABLE_ALL)
    addLayerType(std::make_unique<ColorReliefLayerFactory>());
#endif
#if !defined(MBGL_LAYER_FILL_EXTRUSION_DISABLE_ALL)
    addLayerType(std::make_unique<FillExtrusionLayerFactory>());
#endif
#if !defined(MBGL_LAYER_HEATMAP_DISABLE_ALL)
    addLayerType(std::make_unique<HeatmapLayerFactory>());
#endif
#ifdef MLN_RENDER_BACKEND_OPENGL
#if !defined(MBGL_LAYER_CUSTOM_DISABLE_ALL)
    addLayerType(std::make_unique<CustomLayerFactory>());
#endif
#endif
#if !defined(MBGL_LAYER_LOCATION_INDICATOR_DISABLE_ALL)
    addLayerType(std::make_unique<LocationIndicatorLayerFactory>());
#endif
#if !defined(MLN_LAYER_CUSTOM_DRAWABLE_DISABLE_ALL)
    addLayerType(std::make_unique<CustomDrawableLayerFactory>());
#endif
}

void LayerManagerDefault::addLayerType(std::unique_ptr<LayerFactory> factory) {
    std::string type{factory->getTypeInfo()->type};
    if (!type.empty()) {
        typeToFactory.emplace(std::make_pair(std::move(type), factory.get()));
    } else {
        Log::Warning(Event::Setup,
                     "Failure adding layer factory. getTypeInfo() returned an empty "
                     "type string.");
    }
    factories.emplace_back(std::move(factory));
}

LayerFactory* LayerManagerDefault::getFactory(const mbgl::style::LayerTypeInfo* typeInfo) noexcept {
    assert(typeInfo);
    for (const auto& factory : factories) {
        if (layerTypeInfoEquals(factory->getTypeInfo(), typeInfo)) {
            return factory.get();
        }
    }
    assert(false);
    return nullptr;
}

LayerFactory* LayerManagerDefault::getFactory(const std::string& type) noexcept {
    auto search = typeToFactory.find(type);
    return (search != typeToFactory.end()) ? search->second : nullptr;
}

// static
LayerManager* LayerManager::get() noexcept {
    static LayerManagerDefault instance;
    return &instance;
}

#if defined(MBGL_LAYER_LINE_DISABLE_ALL) || defined(MBGL_LAYER_SYMBOL_DISABLE_ALL) || \
    defined(MBGL_LAYER_FILL_DISABLE_ALL)
const bool LayerManager::annotationsEnabled = false;
#else
const bool LayerManager::annotationsEnabled = true;
#endif

} // namespace mbgl
