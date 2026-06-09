//
//  PluginManager.hpp
//  App
//
//  Created by Malcolm Toon on 11/26/25.
//

#ifndef PluginManager_hpp
#define PluginManager_hpp

#include <stdio.h>
#include <memory>
#include <vector>
#include <mbgl/plugin/plugin.hpp>
#include <mbgl/plugin/plugin_style_preprocessor.hpp>
#include <mbgl/plugin/plugin_map_layer.hpp>

namespace mbgl { namespace plugin {

    class PluginManager {
    public:
        
        virtual ~PluginManager();

        // Plugin manager is a singleton
        static PluginManager* get() noexcept;

        // Add a plugin map layer.  Since layers are instantiated as part of the style load
        // this will add effectively a factory that can create a layer at runtime
        void addMapLayerType(std::shared_ptr<MapLayerType> mapLayerType);
        
        // Add a style pre-processor
        void addStylePreprocessor(std::shared_ptr<StylePreprocessor>);
        
        // Return a list of the style preprocessors.  This is done this way
        // to create a copy of them to iterate so in case something is added or removed
        // during the pre-process, it doesn't affect this
        std::vector<std::shared_ptr<StylePreprocessor>> getStylePreprocessors();
        
    private:
        std::vector<std::shared_ptr<MapLayerType>> mapLayers;
        std::vector<std::shared_ptr<StylePreprocessor>> stylePreprocessors;

    };

} }

#endif /* PluginManager_hpp */
