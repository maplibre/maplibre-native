#include <mbgl/style/layers/debug_layer_impl.hpp>

namespace mbgl {
namespace style {

DebugLayer::Impl::Impl(const Impl& other)
    : Layer::Impl(other) {}

bool DebugLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

} // namespace style
} // namespace mbgl
