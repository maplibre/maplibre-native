#include <mbgl/layermanager/color_relief_layer_factory.hpp>

#include <mbgl/renderer/layers/render_color_relief_layer.hpp>
#include <mbgl/style/layers/color_relief_layer.hpp>
#include <mbgl/style/layers/color_relief_layer_impl.hpp>

namespace mbgl {

const style::LayerTypeInfo* ColorReliefLayerFactory::getTypeInfo() const noexcept {
    return style::ColorReliefLayer::Impl::staticTypeInfo();
}

std::unique_ptr<style::Layer> ColorReliefLayerFactory::createLayer(const std::string& id,
                                                                   const style::conversion::Convertible& value) noexcept {
    const auto source = getSource(value);
    if (!source) {
        return nullptr;
    }
    return std::unique_ptr<style::Layer>(new (std::nothrow) style::ColorReliefLayer(id, *source));
}

std::unique_ptr<RenderLayer> ColorReliefLayerFactory::createRenderLayer(Immutable<style::Layer::Impl> impl) noexcept {
    assert(impl->getTypeInfo() == getTypeInfo());
    return std::make_unique<RenderColorReliefLayer>(staticImmutableCast<style::ColorReliefLayer::Impl>(impl));
}

} // namespace mbgl
