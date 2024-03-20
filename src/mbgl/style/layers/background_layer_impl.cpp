#include <mbgl/style/layers/background_layer_impl.hpp>

namespace mbgl {
namespace style {

BackgroundLayer::Impl::Impl(const Impl& other)
    : Layer::Impl(other),
      paint(other.paint) {}

bool BackgroundLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

} // namespace style
} // namespace mbgl
