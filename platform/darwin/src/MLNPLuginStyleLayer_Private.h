#pragma once

#include "MLNStyleLayer_Private.h"

#include <mbgl/plugins/plugin_layer_factory.hpp>

namespace mbgl {

class PluginLayerPeerFactory : public LayerPeerFactory, public mbgl::PluginLayerFactory {
public:
    PluginLayerPeerFactory(std::string& layerType,
                           mbgl::style::LayerTypeInfo::Source source,
                           mbgl::style::LayerTypeInfo::Pass3D pass3D,
                           mbgl::style::LayerTypeInfo::Layout layout,
                           mbgl::style::LayerTypeInfo::FadingTiles fadingTiles,
                           mbgl::style::LayerTypeInfo::CrossTileIndex crossTileIndex,
                           mbgl::style::LayerTypeInfo::TileKind tileKind)
        : mbgl::PluginLayerFactory(layerType, source, pass3D, layout, fadingTiles, crossTileIndex, tileKind) {}

    // LayerPeerFactory overrides.
    LayerFactory* getCoreLayerFactory() final { return this; }
    virtual MLNStyleLayer* createPeer(style::Layer*) final;
};

} // namespace mbgl
