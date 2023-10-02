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

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/context.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#endif

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

void RenderCustomDrawableLayer::markContextDestroyed() {
    contextDestroyed = true;
}

void RenderCustomDrawableLayer::prepare(const LayerPrepareParameters&) {}

#if MLN_LEGACY_RENDERER
void RenderCustomDrawableLayer::render(PaintParameters& paintParameters) {
}
#endif

#if MLN_DRAWABLE_RENDERER
void RenderCustomDrawableLayer::update([[maybe_unused]] gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               [[maybe_unused]] const TransformState& state,
                               const std::shared_ptr<UpdateParameters>&,
                               [[maybe_unused]] const RenderTree& renderTree,
                               [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    // create layer group
    if (!layerGroup) {
        if (auto layerGroup_ = context.createLayerGroup(layerIndex, /*initialCapacity=*/1, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        }
    }

    // auto* localLayerGroup = static_cast<LayerGroup*>(layerGroup.get());

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

    // // create drawable
    // if (localLayerGroup->getDrawableCount() == 0 || hostChanged) {
    //     localLayerGroup->clearDrawables();

    //     // create tweaker
    //     auto tweaker = std::make_shared<gfx::DrawableCustomLayerHostTweaker>(host);

    //     // create empty drawable using a builder
    //     std::unique_ptr<gfx::DrawableBuilder> builder = context.createDrawableBuilder(getID());
    //     auto& drawable = builder->getCurrentDrawable(true);
    //     drawable->setIsCustom(true);
    //     drawable->setRenderPass(RenderPass::Translucent);

    //     // assign tweaker to drawable
    //     drawable->addTweaker(tweaker);

    //     // add drawable to layer group
    //     localLayerGroup->addDrawable(std::move(drawable));
    //     ++stats.drawablesAdded;
    // }
}
#endif

} // namespace mbgl
