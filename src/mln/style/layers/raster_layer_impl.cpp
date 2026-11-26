#include <mln/style/layers/raster_layer_impl.hpp>

namespace mln {
namespace style {

bool RasterLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

} // namespace style
} // namespace mln
