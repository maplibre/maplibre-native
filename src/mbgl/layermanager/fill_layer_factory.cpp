#include <mbgl/layermanager/fill_layer_factory.hpp>

#include <mbgl/renderer/buckets/fill_bucket.hpp>
#include <mbgl/renderer/layers/render_fill_layer.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>

namespace mbgl {

const style::LayerTypeInfo* FillLayerFactory::getTypeInfo() const noexcept {
    return style::FillLayer::Impl::staticTypeInfo();
}

std::unique_ptr<style::Layer> FillLayerFactory::createLayer(const std::string& id,
                                                            const style::conversion::Convertible& value) noexcept {
    const auto source = getSource(value);
    return std::unique_ptr<style::Layer>(source ? new (std::nothrow) style::FillLayer(id, *source) : nullptr);
}

std::unique_ptr<Layout> FillLayerFactory::createLayout(const LayoutParameters& parameters,
                                                       std::unique_ptr<GeometryTileLayer> layer,
                                                       const std::vector<Immutable<style::LayerProperties>>& group) {
    using namespace style;
    using LayoutTypeUnsorted = PatternLayout<FillBucket, FillLayerProperties, FillPattern, FillLayoutProperties>;
    using LayoutTypeSorted =
        PatternLayout<FillBucket, FillLayerProperties, FillPattern, FillLayoutProperties, FillSortKey>;
    auto layerProperties = staticImmutableCast<FillLayerProperties>(group.front());
    if (layerProperties->layerImpl().layout.get<FillSortKey>().isUndefined()) {
        return std::unique_ptr<Layout>(
            new (std::nothrow) LayoutTypeUnsorted(parameters.bucketParameters, group, std::move(layer), parameters));
    }
    return std::unique_ptr<Layout>(
        new (std::nothrow) LayoutTypeSorted(parameters.bucketParameters, group, std::move(layer), parameters));
}

std::unique_ptr<RenderLayer> FillLayerFactory::createRenderLayer(Immutable<style::Layer::Impl> impl) noexcept {
    assert(impl->getTypeInfo() == getTypeInfo());
    auto fillImpl = staticImmutableCast<style::FillLayer::Impl>(impl);
    return std::unique_ptr<RenderLayer>(new (std::nothrow) RenderFillLayer(std::move(fillImpl)));
}

} // namespace mbgl
