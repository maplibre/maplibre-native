#include <mln/style/layers/color_relief_layer_impl.hpp>

namespace mln {
namespace style {

bool ColorReliefLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

} // namespace style
} // namespace mln
