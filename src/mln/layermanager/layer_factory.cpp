#include <mln/layermanager/layer_factory.hpp>

#include <mln/layout/layout.hpp>
#include <mln/renderer/bucket.hpp>
#include <mln/renderer/bucket_parameters.hpp>

#include <mln/style/conversion/constant.hpp>
#include <mln/style/conversion/filter.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/layer.hpp>

namespace mln {

std::optional<std::string> LayerFactory::getSource(const style::conversion::Convertible& value) const noexcept {
    auto sourceValue = objectMember(value, "source");
    if (!sourceValue) {
        return std::nullopt;
    }

    auto source = toString(*sourceValue);
    if (!source) {
        return std::nullopt;
    }

    return source;
}

std::unique_ptr<Bucket> LayerFactory::createBucket(const BucketParameters&,
                                                   const std::vector<Immutable<style::LayerProperties>>&) noexcept {
    assert(false);
    return nullptr;
}

std::unique_ptr<Layout> LayerFactory::createLayout(const LayoutParameters&,
                                                   std::unique_ptr<GeometryTileLayer>,
                                                   const std::vector<Immutable<style::LayerProperties>>&) {
    assert(false);
    return nullptr;
}

} // namespace mln
