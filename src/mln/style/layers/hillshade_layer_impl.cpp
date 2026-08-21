#include <mln/style/layers/hillshade_layer_impl.hpp>

namespace mln {
namespace style {

bool HillshadeLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

} // namespace style
} // namespace mln
