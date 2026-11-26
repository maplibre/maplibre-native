#include <mln/style/layer_impl.hpp>

namespace mln {
namespace style {

Layer::Impl::Impl(std::string layerID, std::string sourceID)
    : id(std::move(layerID)),
      source(std::move(sourceID)) {}

void Layer::Impl::populateFontStack(std::set<FontStack>&) const {}

} // namespace style
} // namespace mln
