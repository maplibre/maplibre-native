#include <mbgl/renderer/layers/render_plugin_style_layer.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/renderer/layer_group.hpp>

namespace mbgl {
namespace {

const style::PluginStyleLayer::Impl& pluginImpl(const Immutable<style::Layer::Impl>& impl) {
    return static_cast<const style::PluginStyleLayer::Impl&>(*impl);
}

RenderPass renderPassFor(mln_plugin_render_stage stage) {
    switch (stage) {
        case MLN_PLUGIN_RENDER_STAGE_PASS_3D:
            return RenderPass::Pass3D;
        case MLN_PLUGIN_RENDER_STAGE_OPAQUE:
            return RenderPass::Opaque;
        case MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT:
            return RenderPass::Translucent;
        default:
            return RenderPass::None;
    }
}

} // namespace

RenderPluginStyleLayer::RenderPluginStyleLayer(Immutable<style::PluginStyleLayer::Impl> impl)
    : RenderLayer(makeMutable<style::PluginStyleLayerProperties>(std::move(impl))) {
    // Unlike generated layers, a plugin layer has no generated evaluated
    // property object that initializes its pass. It must be renderable from
    // its first frame, including styles installed before the initial zoom
    // evaluation.
    passes = renderPassFor(pluginImpl(baseImpl).registration.renderStage);
}

void RenderPluginStyleLayer::evaluate(const PropertyEvaluationParameters&) {
    passes = renderPassFor(pluginImpl(baseImpl).registration.renderStage);
}

bool RenderPluginStyleLayer::is3D() const {
    return pluginImpl(baseImpl).registration.requires3D;
}

void RenderPluginStyleLayer::update(gfx::ShaderRegistry&,
                                    gfx::Context& context,
                                    const TransformState&,
                                    const std::shared_ptr<UpdateParameters>&,
                                    const RenderTree&,
                                    UniqueChangeRequestVec& changes) {
    if (!layerGroup) {
        if (auto group = context.createLayerGroup(layerIndex, 0, getID())) {
            setLayerGroup(std::move(group), changes);
        }
    }
}

} // namespace mbgl
