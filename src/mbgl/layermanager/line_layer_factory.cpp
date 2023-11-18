#include <mbgl/layermanager/line_layer_factory.hpp>

#include <mbgl/renderer/buckets/line_bucket.hpp>
#include <mbgl/renderer/layers/render_line_layer.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>

namespace mbgl {

const style::LayerTypeInfo* LineLayerFactory::getTypeInfo() const noexcept {
    return style::LineLayer::Impl::staticTypeInfo();
}

std::unique_ptr<style::Layer> LineLayerFactory::createLayer(const std::string& id,
                                                            const style::conversion::Convertible& value) noexcept {
    const auto source = getSource(value);
    return std::unique_ptr<style::Layer>(source ? new (std::nothrow) style::LineLayer(id, *source) : nullptr);
}

std::unique_ptr<Layout> LineLayerFactory::createLayout(
    const LayoutParameters& parameters,
    std::unique_ptr<GeometryTileLayer> layer,
    const std::vector<Immutable<style::LayerProperties>>& group) noexcept {
    using namespace style;
    using LayoutTypeUnsorted = PatternLayout<LineBucket, LineLayerProperties, LinePattern, LineLayoutProperties>;
    using LayoutTypeSorted =
        PatternLayout<LineBucket, LineLayerProperties, LinePattern, LineLayoutProperties, LineSortKey>;
    auto layerProperties = staticImmutableCast<LineLayerProperties>(group.front());
    if (layerProperties->layerImpl().layout.get<LineSortKey>().isUndefined()) {
        return std::unique_ptr<Layout>(
            new (std::nothrow) LayoutTypeUnsorted(parameters.bucketParameters, group, std::move(layer), parameters));
    }
    return std::unique_ptr<Layout>(
        new (std::nothrow) LayoutTypeSorted(parameters.bucketParameters, group, std::move(layer), parameters));
}

std::unique_ptr<RenderLayer> LineLayerFactory::createRenderLayer(Immutable<style::Layer::Impl> impl) noexcept {
    assert(impl->getTypeInfo() == getTypeInfo());
    auto lineImpl = staticImmutableCast<style::LineLayer::Impl>(impl);
    return std::unique_ptr<RenderLayer>(new (std::nothrow) RenderLineLayer(std::move(lineImpl)));
}

} // namespace mbgl
