#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/layermanager/custom_drawable_layer_factory.hpp>
#include <mbgl/style/layers/custom_drawable_layer_impl.hpp>
#include <mbgl/renderer/layers/render_custom_drawable_layer.hpp>

namespace mbgl {

const style::LayerTypeInfo* CustomDrawableLayerFactory::getTypeInfo() const noexcept {
    return style::CustomDrawableLayer::Impl::staticTypeInfo();
}

std::unique_ptr<style::Layer> CustomDrawableLayerFactory::createLayer(const std::string&,
                                                                      const style::conversion::Convertible&) noexcept {
    assert(false);
    return nullptr;
}

std::unique_ptr<RenderLayer> CustomDrawableLayerFactory::createRenderLayer(
    Immutable<style::Layer::Impl> impl) noexcept {
    return std::make_unique<RenderCustomDrawableLayer>(staticImmutableCast<style::CustomDrawableLayer::Impl>(impl));
}

} // namespace mbgl
