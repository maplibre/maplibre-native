#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/style/layers/custom_drawable_layer_impl.hpp>
#include <mbgl/renderer/layers/render_custom_drawable_layer.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/mat4.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/gfx/drawable_builder.hpp>

namespace mbgl {

using namespace style;

namespace {

inline const CustomDrawableLayer::Impl& impl(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == CustomDrawableLayer::Impl::staticTypeInfo());
    return static_cast<const CustomDrawableLayer::Impl&>(*impl);
}

} // namespace

RenderCustomDrawableLayer::RenderCustomDrawableLayer(Immutable<style::CustomDrawableLayer::Impl> _impl)
    : RenderLayer(makeMutable<CustomDrawableLayerProperties>(std::move(_impl))),
      host(impl(baseImpl).host) {
    assert(gfx::BackendScope::exists());
    host->initialize();
}

RenderCustomDrawableLayer::~RenderCustomDrawableLayer() {
    assert(gfx::BackendScope::exists());
    host->deinitialize();
}

void RenderCustomDrawableLayer::evaluate(const PropertyEvaluationParameters&) {
    passes = RenderPass::Translucent;
    // It is fine to not update `evaluatedProperties`, as `baseImpl` should never be updated for this layer.
}

bool RenderCustomDrawableLayer::hasTransition() const {
    return false;
}
bool RenderCustomDrawableLayer::hasCrossfade() const {
    return false;
}

void RenderCustomDrawableLayer::prepare(const LayerPrepareParameters&) {}

void RenderCustomDrawableLayer::update(gfx::ShaderRegistry& shaders,
                                       gfx::Context& context,
                                       const TransformState& state,
                                       const std::shared_ptr<UpdateParameters>& updateParameters,
                                       const RenderTree& renderTree,
                                       UniqueChangeRequestVec& changes) {
    // check if host changed and update
    bool hostChanged = (host != impl(baseImpl).host);
    if (hostChanged) {
        // deinitialize the previous one before initializing the new one.
        if (host) {
            host->deinitialize();
        }
        host = impl(baseImpl).host;
        host->initialize();
    }

    // delegate the call to the custom layer
    if (host) {
        CustomDrawableLayerHost::Interface interface(
            *this, layerGroup, shaders, context, state, updateParameters, renderTree, changes);
        host->update(interface);
    }
}

} // namespace mbgl
