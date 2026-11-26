#pragma once

#include "MLNStyleLayer_Private.h"

#include <mln/layermanager/custom_layer_factory.hpp>

namespace mln {

class CustomStyleLayerPeerFactory : public LayerPeerFactory, public mln::CustomLayerFactory {
    // LayerPeerFactory overrides.
    LayerFactory* getCoreLayerFactory() final { return this; }
    virtual MLNStyleLayer* createPeer(style::Layer*) final;
};

} // namespace mln
