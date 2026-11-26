#include <mln/style/layers/fill_extrusion_layer_impl.hpp>

namespace mln {
namespace style {

bool FillExtrusionLayer::Impl::hasLayoutDifference(const Layer::Impl& other) const {
    assert(other.getTypeInfo() == getTypeInfo());
    const auto& impl = static_cast<const style::FillExtrusionLayer::Impl&>(other);
    return filter != impl.filter || visibility != impl.visibility || paint.hasDataDrivenPropertyDifference(impl.paint);
}

} // namespace style
} // namespace mln
