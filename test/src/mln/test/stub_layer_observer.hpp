#pragma once

#include <mln/style/layer_observer.hpp>

using namespace mln;
using namespace mln::style;

/**
 * An implementation of style::LayerObserver that forwards all methods to
 * dynamically-settable lambdas.
 */
class StubLayerObserver : public style::LayerObserver {
public:
    void onLayerChanged(Layer& layer) override {
        if (layerChanged) layerChanged(layer);
    }

    std::function<void(Layer&)> layerChanged;
};
