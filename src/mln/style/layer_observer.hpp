#pragma once

namespace mln {
namespace style {

class Layer;

class LayerObserver {
public:
    virtual ~LayerObserver() = default;

    virtual void onLayerChanged(Layer&) {}
};

} // namespace style
} // namespace mln
