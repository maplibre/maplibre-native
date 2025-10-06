#pragma once

#include <mbgl/layermanager/custom_drawable_layer_factory.hpp>

#include "MLNStyleLayer_Private.h"

namespace mbgl {

class CustomDrawableStyleLayerPeerFactory
    : public LayerPeerFactory,
      public mbgl::CustomDrawableLayerFactory {
  // LayerPeerFactory overrides.
  LayerFactory* getCoreLayerFactory() final { return this; }
  virtual MLNStyleLayer* createPeer(style::Layer*) final;
};

}  // namespace mbgl
