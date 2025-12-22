//
//  PluginMapLayer.cpp
//  App
//
//  Created by Malcolm Toon on 11/26/25.
//

#include <mbgl/plugin/plugin_map_layer.hpp>

using namespace mbgl::plugin;

__attribute__((visibility("default")))
MapLayerType::MapLayerType() {
    
}

__attribute__((visibility("default")))
__attribute__((used))
MapLayerType::~MapLayerType() = default;

// Returns the layer type string that is used in the style
__attribute__((used))
__attribute__((visibility("default")))
std::string MapLayerType::getLayerType() {
    // Base class does nothing
    return "base class does nothing";
}

// If this layer type requires pass 3d
__attribute__((used))
__attribute__((visibility("default")))
bool mbgl::plugin::MapLayerType::requiresPass3D() {
    // Base class returns false
    return false;
}

// The list of properties
__attribute__((used))
__attribute__((visibility("default")))
std::vector<std::shared_ptr<mbgl::plugin::LayerProperty>> mbgl::plugin::MapLayerType::getLayerProperties() {
    std::vector<std::shared_ptr<LayerProperty>> tempResult;
    return tempResult;
}

// This creates the actual map layer.  Should be overridden by the
// implementor and return a class descended from the MapLayer below
__attribute__((used))
__attribute__((visibility("default")))
std::shared_ptr<mbgl::plugin::MapLayer> mbgl::plugin::MapLayerType::createMapLayer() {
    // Base class does nothing
    return nullptr;
}



/*
MapLayerType::~MapLayerType() {
    // Base class does nothing
}
*/

//void MapLayerType::doSomething() {
//    
//}




void MapLayer::onRender([[maybe_unused]] const RenderingContext *context) {
    // Base class does nothing
}

void MapLayer::onAddedToMap() {
    // Base class does nothing
}

void MapLayer::onUpdate([[maybe_unused]] const DrawingContext &context) {
    // Base class does nothing
}

void MapLayer::onUpdateLayerProperties([[maybe_unused]] const std::string& properties) {
    // Base class does nothing
}


void MapLayer::onMemoryReductionEvent() {
    // Base class does nothing
}

MapLayer::~MapLayer() {
    
}


//int main() {
//    MapLayerType::doSomething();
//}


extern "C"
__attribute__((used))
__attribute__((visibility("default")))
void _force_link_MapLayerType() {
    (void)sizeof(MapLayerType);
}
