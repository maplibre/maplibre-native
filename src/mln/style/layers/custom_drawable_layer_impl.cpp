#include <mln/style/layers/custom_drawable_layer_impl.hpp>

namespace mln {
namespace style {

CustomDrawableLayer::Impl::Impl(const std::string& id_, std::unique_ptr<CustomDrawableLayerHost> host_)
    : Layer::Impl(id_, std::string()) {
    host = std::move(host_);
}

bool CustomDrawableLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

void CustomDrawableLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const {}

} // namespace style
} // namespace mln
