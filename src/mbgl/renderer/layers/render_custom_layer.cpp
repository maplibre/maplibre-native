#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/style/layers/custom_layer_impl.hpp>
#include <mbgl/renderer/layers/render_custom_layer.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/mat4.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/gfx/drawable_custom_layer_host_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>

// TODO: platform agnostic error checks
#define MBGL_CHECK_ERROR(cmd) (cmd)

namespace mbgl {

using namespace style;

namespace {

inline const CustomLayer::Impl& impl(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == CustomLayer::Impl::staticTypeInfo());
    return static_cast<const CustomLayer::Impl&>(*impl);
}

} // namespace

RenderCustomLayer::RenderCustomLayer(Immutable<style::CustomLayer::Impl> _impl)
    : RenderLayer(makeMutable<CustomLayerProperties>(std::move(_impl))),
      host(impl(baseImpl).host) {
    assert(gfx::BackendScope::exists());
    MBGL_CHECK_ERROR(host->initialize());
}

RenderCustomLayer::~RenderCustomLayer() {
    assert(gfx::BackendScope::exists());
    if (contextDestroyed) {
        host->contextLost();
    } else {
        MBGL_CHECK_ERROR(host->deinitialize());
    }
}

void RenderCustomLayer::evaluate(const PropertyEvaluationParameters&) {
    passes = RenderPass::Translucent;
    // It is fine to not update `evaluatedProperties`, as `baseImpl` should never be updated for this layer.
}

bool RenderCustomLayer::hasTransition() const {
    return false;
}
bool RenderCustomLayer::hasCrossfade() const {
    return false;
}

void RenderCustomLayer::markContextDestroyed() {
    contextDestroyed = true;
}

void RenderCustomLayer::prepare(const LayerPrepareParameters&) {}

void RenderCustomLayer::update([[maybe_unused]] gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               [[maybe_unused]] const TransformState& state,
                               const std::shared_ptr<UpdateParameters>&,
                               [[maybe_unused]] const RenderTree& renderTree,
                               [[maybe_unused]] UniqueChangeRequestVec& changes) {
    // create layer group
    if (!layerGroup) {
        if (auto layerGroup_ = context.createLayerGroup(layerIndex, /*initialCapacity=*/1, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        }
    }

    auto* localLayerGroup = static_cast<LayerGroup*>(layerGroup.get());

    // check if host changed and update
    bool hostChanged = (host != impl(baseImpl).host);
    if (hostChanged) {
        // If the context changed, deinitialize the previous one before initializing the new one.
        if (host && !contextDestroyed) {
            MBGL_CHECK_ERROR(host->deinitialize());
        }
        host = impl(baseImpl).host;
        MBGL_CHECK_ERROR(host->initialize());
    }

    // create drawable
    if (localLayerGroup->getDrawableCount() == 0 || hostChanged) {
        localLayerGroup->clearDrawables();

        // create tweaker
        auto tweaker = std::make_shared<gfx::DrawableCustomLayerHostTweaker>(host);

        // create empty drawable using a builder
        std::unique_ptr<gfx::DrawableBuilder> builder = context.createDrawableBuilder(getID());
        auto& drawable = builder->getCurrentDrawable(true);
        drawable->setIsCustom(true);
        drawable->setRenderPass(RenderPass::Translucent);

        // assign tweaker to drawable
        drawable->addTweaker(tweaker);

        // add drawable to layer group
        localLayerGroup->addDrawable(std::move(drawable));
        ++stats.drawablesAdded;
    }
}

} // namespace mbgl
