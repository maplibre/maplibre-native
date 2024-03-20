#include <mbgl/layermanager/raster_layer_factory.hpp>

#include <mbgl/renderer/layers/render_raster_layer.hpp>
#include <mbgl/style/layers/raster_layer.hpp>
#include <mbgl/style/layers/raster_layer_impl.hpp>

namespace mbgl {

const style::LayerTypeInfo* RasterLayerFactory::getTypeInfo() const noexcept {
    return style::RasterLayer::Impl::staticTypeInfo();
}

std::unique_ptr<style::Layer> RasterLayerFactory::createLayer(const std::string& id,
                                                              const style::conversion::Convertible& value) noexcept {
    const auto source = getSource(value);
    if (!source) {
        return nullptr;
    }
    return std::unique_ptr<style::Layer>(new (std::nothrow) style::RasterLayer(id, *source));
}

std::unique_ptr<RenderLayer> RasterLayerFactory::createRenderLayer(Immutable<style::Layer::Impl> impl) noexcept {
    assert(impl->getTypeInfo() == getTypeInfo());
    return std::make_unique<RenderRasterLayer>(staticImmutableCast<style::RasterLayer::Impl>(impl));
}

} // namespace mbgl
