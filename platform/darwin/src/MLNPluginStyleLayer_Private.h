#pragma once

#include "MLNStyleLayer_Private.h"

#include <mln/plugin/plugin_layer_factory.hpp>

namespace mln {

class PluginLayerPeerFactory : public LayerPeerFactory, public mln::PluginLayerFactory {
public:
    PluginLayerPeerFactory(std::string& layerType,
                           mln::style::LayerTypeInfo::Source source,
                           mln::style::LayerTypeInfo::Pass3D pass3D,
                           mln::style::LayerTypeInfo::Layout layout,
                           mln::style::LayerTypeInfo::FadingTiles fadingTiles,
                           mln::style::LayerTypeInfo::CrossTileIndex crossTileIndex,
                           mln::style::LayerTypeInfo::TileKind tileKind)
        : mln::PluginLayerFactory(layerType, source, pass3D, layout, fadingTiles, crossTileIndex, tileKind) {}

    // LayerPeerFactory overrides.
    LayerFactory* getCoreLayerFactory() final { return this; }
    virtual MLNStyleLayer* createPeer(style::Layer*) final;
};

} // namespace mln
