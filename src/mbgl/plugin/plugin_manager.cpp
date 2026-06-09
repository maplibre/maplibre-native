//
//  PluginManager.cpp
//  App
//
//  Created by Malcolm Toon on 11/26/25.
//

#include "plugin_manager.hpp"
#include <mbgl/plugin/plugin_style_filter.hpp>
#include <mbgl/plugin/plugin_layer_impl.hpp>
#include <mbgl/plugin/plugin_layer_factory.hpp>
#include <mbgl/layermanager/layer_manager.hpp>

using namespace mbgl::plugin;
using namespace mbgl::style;

PluginManager::~PluginManager() = default;

PluginManager* PluginManager::get() noexcept {
    static PluginManager manager;
    return &manager;

}

std::vector<std::shared_ptr<StylePreprocessor>> PluginManager::getStylePreprocessors() {
    return stylePreprocessors;
}



void PluginManager::addStylePreprocessor(std::shared_ptr<StylePreprocessor> stylePreprocessor) {
    
    stylePreprocessors.push_back(stylePreprocessor);
    
}

void PluginManager::addMapLayerType(std::shared_ptr<MapLayerType> mapLayerType) {
    
    auto layerManager = mbgl::LayerManager::get();
    std::string layerType = mapLayerType->getLayerType();

    // Default values
    auto source = mbgl::style::LayerTypeInfo::Source::NotRequired;
    auto tileKind = mbgl::style::LayerTypeInfo::TileKind::NotRequired;
    auto fadingTiles = mbgl::style::LayerTypeInfo::FadingTiles::NotRequired;
    auto layout = mbgl::style::LayerTypeInfo::Layout::NotRequired;
    auto crossTileIndex = mbgl::style::LayerTypeInfo::CrossTileIndex::NotRequired;
    auto pass3D = mbgl::style::LayerTypeInfo::Pass3D::NotRequired;
    if (mapLayerType->requiresPass3D()) {
        pass3D = mbgl::style::LayerTypeInfo::Pass3D::Required;
    }

    
    auto factory = std::make_unique<mbgl::PluginLayerFactory>(layerType,
                                               source,
                                               pass3D,
                                               layout,
                                               fadingTiles,
                                               crossTileIndex,
                                               tileKind);

    factory->setOnLayerCreatedEvent([mapLayerType](mbgl::style::PluginLayer *pluginLayer) {

        
        auto mapLayer = mapLayerType->createMapLayer();
        
        auto pluginLayerImpl = (mbgl::style::PluginLayer::Impl *)pluginLayer->baseImpl.get();
        auto & pm = pluginLayerImpl->_propertyManager;
        
        auto layerProperties = mapLayerType->getLayerProperties();
        
        for (auto p: layerProperties) {
            mbgl::style::PluginLayerProperty *pp = new mbgl::style::PluginLayerProperty();
            switch (p->propertyType) {
                case LayerProperty::PropertyType::SingleFloat:
                    pp->_propertyType = mbgl::style::PluginLayerProperty::PropertyType::SingleFloat;
                    pp->_defaultSingleFloatValue = p->singleFloatDefaultValue;
                    break;
                case LayerProperty::PropertyType::Color:
                {
                    pp->_propertyType = mbgl::style::PluginLayerProperty::PropertyType::Color;
                    
                    if (p->hasDefaultColorValue) {
                        pp->_defaultColorValue = mbgl::Color(p->colorDefaultValue[0],
                                                             p->colorDefaultValue[1],
                                                             p->colorDefaultValue[2],
                                                             p->colorDefaultValue[3]);
                    }
                }
                    break;
                default:
                    pp->_propertyType = mbgl::style::PluginLayerProperty::PropertyType::Unknown;
                    break;
            }
            pp->_propertyName = p->propertyName;
            pm.addProperty(pp);
        }
        
        // Set the render function
        auto renderFunction = [mapLayer](mbgl::PaintParameters& paintParameters){
            
            
            const mbgl::TransformState& state = paintParameters.state;

            DrawingContext drawingContext;
            drawingContext.drawableSize = {(int)state.getSize().width, (int)state.getSize().height};
            drawingContext.centerCoordinate = {state.getLatLng().latitude(), state.getLatLng().longitude()};
            drawingContext.zoomLevel = state.getZoom();
            drawingContext.direction = mbgl::util::rad2deg(-state.getBearing());
            drawingContext.pitch = state.getPitch();
            drawingContext.fieldOfView = state.getFieldOfView();
            drawingContext.projectionMatrix = paintParameters.transformParams.projMatrix;
            drawingContext.nearClippedProjMatrix = paintParameters.transformParams.nearClippedProjMatrix;
            
            // Call update with the scene state variables
            mapLayer->onUpdate(drawingContext);
           
            // Call render
            auto renderingContext = createPlatformRenderingContext(paintParameters);
            mapLayer->onRender(renderingContext);
            
       
        };

        // Set the lambdas
        //auto pluginLayerImpl = (mbgl::style::PluginLayer::Impl *)pluginLayer->baseImpl.get();
        pluginLayerImpl->setRenderFunction(renderFunction);

        // Set the update properties function
        pluginLayerImpl->setUpdatePropertiesFunction([mapLayer](const std::string & jsonProperties) {
            
            mapLayer->onUpdateLayerProperties(jsonProperties);
            
        });

    });

    // Add the layer type
    layerManager->addLayerTypeCoreOnly(std::move(factory));

    
    mapLayers.push_back(mapLayerType);
    
}
