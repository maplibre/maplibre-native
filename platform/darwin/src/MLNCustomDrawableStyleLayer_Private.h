#pragma once

#include "MLNStyleLayer_Private.h"

#include <mln/layermanager/custom_drawable_layer_factory.hpp>

namespace mln {

class CustomDrawableStyleLayerPeerFactory : public LayerPeerFactory, public mln::CustomDrawableLayerFactory {
    // LayerPeerFactory overrides.
    LayerFactory* getCoreLayerFactory() final { return this; }
    virtual MLNStyleLayer* createPeer(style::Layer*) final;
};

} // namespace mln
