#pragma once

#include "MLNStyleLayer_Private.h"

#include <mbgl/layermanager/custom_layer_factory.hpp>

namespace mbgl {
    
class CustomStyleLayerPeerFactory : public LayerPeerFactory, public mbgl::CustomLayerFactory {
    // LayerPeerFactory overrides.
    LayerFactory* getCoreLayerFactory() final { return this; }
    virtual MLNStyleLayer* createPeer(style::Layer*) final;
};
    
}  // namespace mbgl
