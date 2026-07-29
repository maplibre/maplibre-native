#pragma once

#ifdef __cplusplus

#if __has_include(<MapLibre/plugin.h>)
#include <MapLibre/plugin.h>
#include <MapLibre/plugin_map_layer.h>
#include <MapLibre/plugin_style_preprocessor.h>
#else
#include "plugin.h"
#include "plugin_map_layer.h"
#include "plugin_style_preprocessor.h"
#endif

#include <memory>
#include <vector>

namespace mbgl {
namespace plugin {

class __attribute__((visibility("default"))) PluginManager {
public:
    virtual ~PluginManager();

    static PluginManager* get() noexcept;
    void addMapLayerType(std::shared_ptr<MapLayerType> mapLayerType);
    void addStylePreprocessor(std::shared_ptr<StylePreprocessor>);
    std::vector<std::shared_ptr<StylePreprocessor>> getStylePreprocessors();

private:
    std::vector<std::shared_ptr<MapLayerType>> mapLayers;
    std::vector<std::shared_ptr<StylePreprocessor>> stylePreprocessors;
};

} // namespace plugin
} // namespace mbgl

#endif
