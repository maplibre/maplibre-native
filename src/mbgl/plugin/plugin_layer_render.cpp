#include <mbgl/plugin/plugin_layer_render.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#if MLN_RENDER_BACKEND_METAL
#include <mbgl/style/layers/mtl/custom_layer_render_parameters.hpp>
#include <mbgl/mtl/render_pass.hpp>
#else
#include <mbgl/style/layers/custom_layer_render_parameters.hpp>
#endif
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/plugin/feature_collection_bucket.hpp>

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

void RenderPluginLayerTweaker::init([[maybe_unused]] mbgl::gfx::Drawable& drawablee) {

};

void RenderPluginLayerTweaker::execute([[maybe_unused]] mbgl::gfx::Drawable& drawable,
                                       [[maybe_unused]] mbgl::PaintParameters& paintParameters) {
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

    _plugInRenderer->callRenderFunction(paintParameters);

    // Reset the view back to our original one, just in case the CustomLayer
    // changed the viewport or Framebuffer.
    paintParameters.backend.getDefaultRenderable().getResource<gfx::RenderableResource>().bind();

    context.setDirtyState();
    context.bindGlobalUniformBuffers(*paintParameters.renderPass);
}

RenderPluginLayer::RenderPluginLayer(Immutable<style::PluginLayer::Impl> _impl)
    : RenderLayer(makeMutable<style::PluginLayerProperties>(std::move(_impl))) {}

RenderPluginLayer::~RenderPluginLayer() = default;

void RenderPluginLayer::markLayerRenderable([[maybe_unused]] bool willRender,
                                            [[maybe_unused]] UniqueChangeRequestVec& changes) {
    isRenderable = true;
}

/// Generate any changes needed by the layer
void RenderPluginLayer::update([[maybe_unused]] gfx::ShaderRegistry& shaderRegistery,
                               [[maybe_unused]] gfx::Context& context,
                               [[maybe_unused]] const TransformState& transformState,
                               [[maybe_unused]] const std::shared_ptr<UpdateParameters>& updateParameters,
                               [[maybe_unused]] const RenderTree& renderTree,
                               [[maybe_unused]] UniqueChangeRequestVec& changes) {
    auto pluginLayer = static_cast<const mbgl::style::PluginLayer::Impl&>(*baseImpl);

    std::vector<OverscaledTileID> removedTiles;
    bool removeAllTiles = ((renderTiles == nullptr) || (renderTiles->empty()));

    // Get list of tiles to remove and then remove them
    for (const auto& currentCollection : _featureCollectionByTile) {
        if (removeAllTiles || !hasRenderTile(currentCollection.first)) {
            removedTiles.push_back(currentCollection.first);
        }
    }
    if (removedTiles.size() > 0) {
        for (auto tileID : removedTiles) {
            auto featureCollection = _featureCollectionByTile[tileID];
            if (pluginLayer._featureCollectionUnloadedFunction) {
                pluginLayer._featureCollectionUnloadedFunction(featureCollection);
            }
            _featureCollectionByTile.erase(tileID);
        }
    }

    if (renderTiles) {
        if (!renderTiles->empty()) {
            auto drawPass = RenderPass::Pass3D;

            // If we're reading feature collections, go through
            // and notify the plugin of any new feature collections
            for (const RenderTile& tile : *renderTiles) {
                const auto& tileID = tile.getOverscaledTileID();

                const auto* optRenderData = getRenderDataForPass(tile, drawPass);
                if (!optRenderData || !optRenderData->bucket || !optRenderData->bucket->hasData()) {
                    removeTile(drawPass, tileID);
                    continue;
                }
                const auto& renderData = *optRenderData;
                auto& bucket = static_cast<FeatureCollectionBucket&>(*renderData.bucket);
                auto featureCollection = bucket._featureCollection;
                if (featureCollection == nullptr) {
                    continue;
                }

                // See if we already have this tile's feature collection
                if (_featureCollectionByTile.contains(tileID)) {
                    continue;
                }

                _featureCollectionByTile[tileID] = featureCollection;

                //                static_cast<const Impl&>(*baseImpl);
                //                auto layer = static_cast<const mbgl::style::PluginLayer::Impl
                //                &>(*renderData.layerProperties->baseImpl);
                if (pluginLayer._featureCollectionLoadedFunction) {
                    if (featureCollection != nullptr) {
                        pluginLayer._featureCollectionLoadedFunction(featureCollection);
                    }
                }
            }
        }
    }

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

void RenderPluginLayer::upload([[maybe_unused]] gfx::UploadPass& uploadPass) {}

void RenderPluginLayer::callRenderFunction(PaintParameters& paintParameters) {
    if (_renderFunction) {
        _renderFunction(paintParameters);
    }
}

void RenderPluginLayer::prepare(const LayerPrepareParameters& layerParameters) {
    // This check is here because base prepare will assert on these and crash
    if (layerParameters.source != nullptr) {
        if (layerParameters.source->isEnabled()) {
            RenderLayer::prepare(layerParameters);
        }
    }

    if (_updateFunction) {
        _updateFunction(layerParameters);
    }
}

// --- Private methods
void RenderPluginLayer::transition([[maybe_unused]] const TransitionParameters& parameters) {
    // Called when switching between styles
}

void RenderPluginLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto i = static_cast<const style::PluginLayer::Impl*>(baseImpl.get());

    auto pm = i->_propertyManager;
    for (auto property : pm.getProperties()) {
        if (property->_propertyType == style::PluginLayerProperty::PropertyType::SingleFloat) {
            auto& f = property->getSingleFloat();
            using Evaluator = typename style::DataDrivenSingleFloatProperty::EvaluatorType;
            auto df = property->_defaultSingleFloatValue;
            auto newF = f.evaluate(Evaluator(parameters, df), parameters.now);
            auto v = newF.constant().value();
            property->setCurrentSingleFloatValue(v);
        } else if (property->_propertyType == style::PluginLayerProperty::PropertyType::Color) {
            auto& f = property->getColor();
            using Evaluator = typename style::DataDrivenColorProperty::EvaluatorType;
            auto df = property->_defaultColorValue;
            auto newF = f.evaluate(Evaluator(parameters, df), parameters.now);
            auto v = newF.constant().value();
            property->setCurrentColorValue(v);
        }
    }

    std::string jsonProperties = pm.propertiesAsJSON();
    i->_updateLayerPropertiesFunction(jsonProperties);

    auto properties = makeMutable<style::PluginLayerProperties>(
        staticImmutableCast<style::PluginLayer::Impl>(baseImpl));
    passes = RenderPass::Pass3D;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);
}

bool RenderPluginLayer::hasTransition() const {
    return true;
}

bool RenderPluginLayer::hasCrossfade() const {
    return false;
}
bool RenderPluginLayer::queryIntersectsFeature([[maybe_unused]] const GeometryCoordinates&,
                                               [[maybe_unused]] const GeometryTileFeature&,
                                               [[maybe_unused]] float,
                                               [[maybe_unused]] const TransformState&,
                                               [[maybe_unused]] float,
                                               [[maybe_unused]] const mat4&,
                                               [[maybe_unused]] const FeatureState&) const {
    return false;
}

void RenderPluginLayer::layerChanged([[maybe_unused]] const TransitionParameters& parameters,
                                     [[maybe_unused]] const Immutable<style::Layer::Impl>& impl,
                                     [[maybe_unused]] UniqueChangeRequestVec& changes) {}

/// Remove all drawables for the tile from the layer group
/// @return The number of drawables actually removed.
std::size_t RenderPluginLayer::removeTile([[maybe_unused]] RenderPass renderPass,
                                          [[maybe_unused]] const OverscaledTileID& tileID) {
    return 0;
}

/// Remove all the drawables for tiles
/// @return The number of drawables actually removed.
std::size_t RenderPluginLayer::removeAllDrawables() {
    return 0;
}
