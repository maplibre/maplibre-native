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

#include <mbgl/style/properties.hpp>
#include "plugin_layer_debug.hpp"

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
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    std::cout << "RenderPluginLayerTweaker::execute\n";
#endif
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

RenderPluginLayer::RenderPluginLayer(Immutable<style::PluginLayer::Impl> _impl)
    : RenderLayer(makeMutable<style::PluginLayerProperties>(std::move(_impl))) {}

RenderPluginLayer::~RenderPluginLayer() = default;

void RenderPluginLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    isRenderable = true;
}

/// Generate any changes needed by the layer
void RenderPluginLayer::update(gfx::ShaderRegistry& shaderRegistery,
                               gfx::Context& context,
                               const TransformState& transformState,
                               const std::shared_ptr<UpdateParameters>& updateParameters,
                               const RenderTree& renderTree,
                               UniqueChangeRequestVec& changes) {
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    std::cout << "RenderPluginLayer::update\n";
#endif

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
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    std::cout << "RenderPluginLayer::upload\n";
#endif
}

void RenderPluginLayer::render(PaintParameters& paintParameters) {
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    std::cout << "RenderPluginLayer::render\n";
#endif
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
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    std::cout << "transition\n";
#endif
}

void RenderPluginLayer::evaluate(const PropertyEvaluationParameters& parameters) {
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    std::cout << "evaluate\n";
#endif

    // auto i = staticImmutableCast<style::PluginLayer::Impl>(baseImpl);
    auto i = static_cast<const style::PluginLayer::Impl*>(baseImpl.get());

    auto pm = i->_propertyManager;
    for (auto property : pm.getProperties()) {
        if (property->_propertyType == style::PluginLayerProperty::PropertyType::SingleFloat) {
            auto& f = property->getSingleFloat();
            using Evaluator = typename style::SingleFloatProperty::EvaluatorType;
            auto df = property->_defaultSingleFloatValue;
            auto newF = f.evaluate(Evaluator(parameters, df), parameters.now);
            auto v = newF.constant().value();
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
            std::cout << "V: " << v << "\n";
#endif
            property->setCurrentSingleFloatValue(v);
        } else if (property->_propertyType == style::PluginLayerProperty::PropertyType::Color) {
#if INCLUDE_DATA_DRIVEN_COLOR_PROPERTY

            auto& f = property->getColor();
            using Evaluator = typename style::DataDrivenColorProperty::EvaluatorType;
            auto df = property->_defaultColorValue;
            auto newF = f.evaluate(Evaluator(parameters, df), parameters.now);
            auto v = newF.constant().value();
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
            std::cout << "V: " << v.stringify() << "\n";
#endif
            property->setCurrentColorValue(v);
#endif
        }
    }

    std::string jsonProperties = pm.propertiesAsJSON();

    i->_updateLayerPropertiesFunction(jsonProperties);
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
#if MLN_PLUGIN_LAYER_LOGGING_ENABLED
    //  std::cout << "RenderPluginLayer::layerChanged\n";
#endif
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
