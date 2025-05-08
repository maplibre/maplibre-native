#include "plugin_layer_render.hpp"
#include <mbgl/gfx/context.hpp>

#include <iostream>

// These all support RenderPluginLayerTweaker in case we want to break this out into a different class
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#if MLN_RENDER_BACKEND_METAL
#include <mbgl/style/layers/mtl/custom_layer_render_parameters.hpp>
#include <mbgl/mtl/render_pass.hpp>
#endif
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/renderer_backend.hpp>

using namespace mbgl;

namespace mbgl {
namespace gfx {
class Drawable;
}
} // namespace mbgl

class RenderPluginLayerTweaker : public mbgl::gfx::DrawableTweaker {
public:
    RenderPluginLayerTweaker(RenderPluginLayer* plugInRenderer)
        : _plugInRenderer(plugInRenderer) {}
    ~RenderPluginLayerTweaker() override = default;

    void init(mbgl::gfx::Drawable&) override;

    void execute(mbgl::gfx::Drawable&, mbgl::PaintParameters&) override;

protected:
    RenderPluginLayer* _plugInRenderer = nullptr;
};

void RenderPluginLayerTweaker::init(mbgl::gfx::Drawable& drawablee) {
   // std::cout << "RenderPluginLayerTweaker::init\n";
};

void RenderPluginLayerTweaker::execute(mbgl::gfx::Drawable& drawable, mbgl::PaintParameters& paintParameters) {
   // std::cout << "RenderPluginLayerTweaker::execute\n";

    // custom drawing
    auto& context = paintParameters.context;
    context.resetState(paintParameters.depthModeForSublayer(0, mbgl::gfx::DepthMaskType::ReadOnly),
                       paintParameters.colorModeForRenderPass());

#if MLN_RENDER_BACKEND_METAL
    const auto& mtlRenderPass = static_cast<mbgl::mtl::RenderPass*>(paintParameters.renderPass.get());
    mtlRenderPass->resetState();

    style::mtl::CustomLayerRenderParameters parameters(paintParameters);
#else
    style::CustomLayerRenderParameters parameters(paintParameters);
#endif

    _plugInRenderer->render(paintParameters);
    // host->render(parameters);

    // Reset the view back to our original one, just in case the CustomLayer
    // changed the viewport or Framebuffer.
    paintParameters.backend.getDefaultRenderable().getResource<gfx::RenderableResource>().bind();

    context.setDirtyState();
    context.bindGlobalUniformBuffers(*paintParameters.renderPass);
}

// RenderHeatmapLayer::RenderHeatmapLayer(Immutable<HeatmapLayer::Impl> _impl)
//     : RenderLayer(makeMutable<HeatmapLayerProperties>(std::move(_impl))),
//       unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
//     styleDependencies = unevaluated.getDependencies();
//     colorRamp = std::make_shared<PremultipliedImage>(Size(256, 1));
// }

RenderPluginLayer::RenderPluginLayer(Immutable<style::PluginLayer::Impl> _impl)
    : RenderLayer(makeMutable<style::PluginLayerProperties>(std::move(_impl))) {}

RenderPluginLayer::~RenderPluginLayer() = default;

void RenderPluginLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
   // std::cout << "markLayerRenderable\n";
    isRenderable = true;
}

/// Generate any changes needed by the layer
void RenderPluginLayer::update(gfx::ShaderRegistry& shaderRegistery,
                               gfx::Context& context,
                               const TransformState& transformState,
                               const std::shared_ptr<UpdateParameters>& updateParameters,
                               const RenderTree& renderTree,
                               UniqueChangeRequestVec& changes) {
    // TODO: What should be implemented here?
    // std::cout << "Update\n";

    // create layer group
    if (!layerGroup) {
        if (auto layerGroup_ = context.createLayerGroup(layerIndex, /*initialCapacity=*/1, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        }
    }

    auto* localLayerGroup = static_cast<LayerGroup*>(layerGroup.get());

    // TODO: Implement this
    bool hostChanged = false;

    // create drawable
    if (localLayerGroup->getDrawableCount() == 0 || hostChanged) {
        localLayerGroup->clearDrawables();

        // create tweaker
        auto tweaker = std::make_shared<RenderPluginLayerTweaker>(this);

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

void RenderPluginLayer::upload(gfx::UploadPass& uploadPass) {
   // std::cout << "Upload\n";
}

void RenderPluginLayer::render(PaintParameters& paintParameters) {
   // std::cout << "Render\n";
    if (_renderFunction) {
        _renderFunction(paintParameters);
    }
}

void RenderPluginLayer::prepare(const LayerPrepareParameters& layerParameters) {
    // TODO: What should be implemented here?
//    std::cout << "prepare\n";
    if (_updateFunction) {
        _updateFunction(layerParameters);
    }

}

// --- Private methods
void RenderPluginLayer::transition(const TransitionParameters& parameters) {
    // Called when switching between styles
   std::cout << "transition\n";
}

void RenderPluginLayer::evaluate(const PropertyEvaluationParameters& parameters) {
   std::cout << "evaluate\n";
}

bool RenderPluginLayer::hasTransition() const {
    return true;
}

bool RenderPluginLayer::hasCrossfade() const {
    return false;
}
bool RenderPluginLayer::queryIntersectsFeature(const GeometryCoordinates&,
                                               const GeometryTileFeature&,
                                               float,
                                               const TransformState&,
                                               float,
                                               const mat4&,
                                               const FeatureState&) const {
    return false;
}

void RenderPluginLayer::layerChanged(const TransitionParameters& parameters,
                                     const Immutable<style::Layer::Impl>& impl,
                                     UniqueChangeRequestVec& changes) {
  //  std::cout << "layerChanged\n";
}

/// Remove all drawables for the tile from the layer group
/// @return The number of drawables actually removed.
std::size_t RenderPluginLayer::removeTile(RenderPass, const OverscaledTileID&) {
    return 0;
}

/// Remove all the drawables for tiles
/// @return The number of drawables actually removed.
std::size_t RenderPluginLayer::removeAllDrawables() {
    return 0;
}
