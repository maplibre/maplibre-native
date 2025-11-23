#include <mbgl/style/layers/color_relief_layer_impl.hpp>

namespace mbgl {
namespace style {

const LayerTypeInfo* ColorReliefLayer::Impl::getTypeInfo() const noexcept {
    return ColorReliefLayer::Impl::staticTypeInfo();
}

bool ColorReliefLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

} // namespace style
} // namespace mbgl
