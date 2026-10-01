#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/style/layers/custom_layer_impl.hpp>
#include <mln/renderer/layers/render_custom_layer.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/math/angles.hpp>
#include <mln/renderer/bucket.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/util/mat4.hpp>

#include <mln/gfx/context.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/gfx/drawable_custom_layer_host_tweaker.hpp>
#include <mln/gfx/drawable_builder.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mln/style/layers/mtl/custom_layer_render_parameters.hpp>
#include <mln/mtl/render_pass.hpp>
#elif MLN_RENDER_BACKEND_VULKAN
#include <mln/vulkan/context.hpp>
#include <mln/vulkan/renderer_backend.hpp>
#include <mln/style/layers/vulkan/custom_layer_init_parameters.hpp>
#include <mln/style/layers/vulkan/custom_layer_render_parameters.hpp>
#include <mln/vulkan/render_pass.hpp>
#endif

// TODO: platform agnostic error checks
#define MBGL_CHECK_ERROR(cmd) (cmd)

namespace mln {

using namespace style;

namespace {

inline const CustomLayer::Impl& impl(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == CustomLayer::Impl::staticTypeInfo());
    return static_cast<const CustomLayer::Impl&>(*impl);
}

void initializeHost(const std::shared_ptr<style::CustomLayerHost>& host, [[maybe_unused]] gfx::Context& context) {
#if MLN_RENDER_BACKEND_VULKAN
    {
        auto& vkBackend = static_cast<mln::vulkan::Context&>(context).getBackend();
        style::vulkan::CustomLayerInitParameters params(
            vkBackend.getDispatcher(), vkBackend.getDevice().get(), vkBackend.getPhysicalDevice());
        host->initialize(params);
    }
#else
    {
        style::CustomLayerInitParameters params;
        host->initialize(params);
    }
#endif
}

void callPreRender(const std::shared_ptr<style::CustomLayerHost>& host,
                   gfx::Context& context,
                   const PaintParameters& paintParameters) {
#if MLN_RENDER_BACKEND_METAL
    style::mtl::CustomLayerRenderParameters parameters(paintParameters);
#elif MLN_RENDER_BACKEND_VULKAN
    style::vulkan::CustomLayerRenderParameters parameters(paintParameters);
#else
    style::CustomLayerRenderParameters parameters(paintParameters);
#endif
    host->preRender(context, parameters);
}

} // namespace

RenderCustomLayer::RenderCustomLayer(Immutable<style::CustomLayer::Impl> _impl)
    : RenderLayer(makeMutable<CustomLayerProperties>(std::move(_impl))),
      host(impl(baseImpl).host) {
    assert(gfx::BackendScope::exists());
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
                               [[maybe_unused]] const PaintParameters& paintParameters,
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
        needsInitialize = true;
    }

    if (needsInitialize) {
        MBGL_CHECK_ERROR(initializeHost(host, context));
        needsInitialize = false;
    }

    // call the pre-render
    MBGL_CHECK_ERROR(callPreRender(host, context, paintParameters));

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

} // namespace mln
