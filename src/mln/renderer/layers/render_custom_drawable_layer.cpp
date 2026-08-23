#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/style/layers/custom_drawable_layer.hpp>
#include <mln/style/layers/custom_drawable_layer_impl.hpp>
#include <mln/renderer/layers/render_custom_drawable_layer.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/math/angles.hpp>
#include <mln/renderer/bucket.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/util/mat4.hpp>

#include <mln/gfx/context.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/gfx/drawable_builder.hpp>

namespace mln {

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
                                       const PaintParameters&,
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

} // namespace mln
