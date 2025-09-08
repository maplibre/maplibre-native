#include <mbgl/layermanager/fill_extrusion_layer_factory.hpp>

#include <mbgl/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mbgl/renderer/layers/render_fill_extrusion_layer.hpp>
#include <mbgl/style/layers/fill_extrusion_layer.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_impl.hpp>

namespace mbgl {

const style::LayerTypeInfo* FillExtrusionLayerFactory::getTypeInfo() const noexcept {
    return style::FillExtrusionLayer::Impl::staticTypeInfo();
}

std::unique_ptr<style::Layer> FillExtrusionLayerFactory::createLayer(
    const std::string& id, const style::conversion::Convertible& value) noexcept {
    const auto source = getSource(value);
    return std::unique_ptr<style::Layer>(source ? new (std::nothrow) style::FillExtrusionLayer(id, *source) : nullptr);
}

std::unique_ptr<Layout> FillExtrusionLayerFactory::createLayout(
    const LayoutParameters& parameters,
    std::unique_ptr<GeometryTileLayer> layer,
    const std::vector<Immutable<style::LayerProperties>>& group) {
    using namespace style;
    using LayoutType = PatternLayout<FillExtrusionBucket, FillExtrusionLayerProperties, FillExtrusionPattern>;
    return std::unique_ptr<Layout>(new (std::nothrow)
                                       LayoutType(parameters.bucketParameters, group, std::move(layer), parameters));
}

std::unique_ptr<RenderLayer> FillExtrusionLayerFactory::createRenderLayer(Immutable<style::Layer::Impl> impl) noexcept {
    assert(impl->getTypeInfo() == getTypeInfo());
    auto renderImpl = staticImmutableCast<style::FillExtrusionLayer::Impl>(impl);
    return std::unique_ptr<RenderLayer>(new (std::nothrow) RenderFillExtrusionLayer(std::move(renderImpl)));
}

} // namespace mbgl
