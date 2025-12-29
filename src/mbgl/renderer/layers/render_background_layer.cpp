#include <mbgl/renderer/layers/render_background_layer.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/pattern_atlas.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/upload_parameters.hpp>
#include <mbgl/style/layers/background_layer_impl.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/util/tile_cover.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>

#include <mbgl/renderer/layers/background_layer_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/shader_program_base.hpp>

#include <algorithm>

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

inline const BackgroundLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == BackgroundLayer::Impl::staticTypeInfo());
    return static_cast<const style::BackgroundLayer::Impl&>(*impl);
}

} // namespace

RenderBackgroundLayer::RenderBackgroundLayer(Immutable<style::BackgroundLayer::Impl> _impl)
    : RenderLayer(makeMutable<BackgroundLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
    styleDependencies = unevaluated.getDependencies();
}

RenderBackgroundLayer::~RenderBackgroundLayer() = default;

void RenderBackgroundLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    styleDependencies = unevaluated.getDependencies();
}

void RenderBackgroundLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto previousProperties = staticImmutableCast<BackgroundLayerProperties>(evaluatedProperties);
    auto evaluated = unevaluated.evaluate(parameters, previousProperties->evaluated);
    auto properties = makeMutable<BackgroundLayerProperties>(
        staticImmutableCast<BackgroundLayer::Impl>(baseImpl), parameters.getCrossfadeParameters(), evaluated);

    passes = properties->evaluated.get<style::BackgroundOpacity>() == 0.0f ? RenderPass::None
             : (!unevaluated.get<style::BackgroundPattern>().isUndefined() ||
                properties->evaluated.get<style::BackgroundOpacity>() < 1.0f ||
                properties->evaluated.get<style::BackgroundColor>().a < 1.0f)
                 ? RenderPass::Translucent
                 // Supply both - evaluated based on opaquePassCutoff in render().
                 : RenderPass::Opaque | RenderPass::Translucent;
    properties->renderPasses = mbgl::underlying_type(passes);

    evaluatedProperties = std::move(properties);

    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
}

bool RenderBackgroundLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderBackgroundLayer::hasCrossfade() const {
    return getCrossfade<BackgroundLayerProperties>(evaluatedProperties).t != 1;
}

std::optional<Color> RenderBackgroundLayer::getSolidBackground() const {
    const auto& evaluated = getEvaluated<BackgroundLayerProperties>(evaluatedProperties);
    if (!evaluated.get<BackgroundPattern>().from.empty() || evaluated.get<style::BackgroundOpacity>() <= 0.0f) {
        return std::nullopt;
    }

    return {evaluated.get<BackgroundColor>() * evaluated.get<BackgroundOpacity>()};
}

namespace {
void addPatternIfNeeded(const std::string& id, const LayerPrepareParameters& params) {
    if (!params.patternAtlas.getPattern(id)) {
        if (auto* image = params.imageManager.getImage(id)) {
            params.patternAtlas.addPattern(*image);
        }
    }
}
} // namespace

void RenderBackgroundLayer::prepare(const LayerPrepareParameters& params) {
    const auto& evaluated = getEvaluated<BackgroundLayerProperties>(evaluatedProperties);
    if (!evaluated.get<BackgroundPattern>().to.empty()) {
        // Ensures that the pattern bitmap gets copied to atlas bitmap.
        // Atlas bitmap is uploaded to atlas texture in upload.
        addPatternIfNeeded(evaluated.get<BackgroundPattern>().from.id(), params);
        addPatternIfNeeded(evaluated.get<BackgroundPattern>().to.id(), params);
    }
}

static constexpr std::string_view BackgroundPlainShaderName = "BackgroundShader";
static constexpr std::string_view BackgroundPatternShaderName = "BackgroundPatternShader";

void RenderBackgroundLayer::update(gfx::ShaderRegistry& shaders,
                                   gfx::Context& context,
                                   const TransformState& state,
                                   const std::shared_ptr<UpdateParameters>& updateParameters,
                                   [[maybe_unused]] const RenderTree& renderTree,
                                   [[maybe_unused]] UniqueChangeRequestVec& changes) {
    assert(updateParameters);
    const auto zoom = state.getIntegerZoom();
    const auto tileCover = util::tileCover({.transformState = state,
                                            .tileLodMinRadius = updateParameters->tileLodMinRadius,
                                            .tileLodScale = updateParameters->tileLodScale,
                                            .tileLodPitchThreshold = updateParameters->tileLodPitchThreshold},
                                           zoom);

    // renderTiles is always empty, we use tileCover instead
    if (tileCover.empty()) {
        removeAllDrawables();
        return;
    }

    const auto& evaluated = getEvaluated<BackgroundLayerProperties>(evaluatedProperties);
    const bool hasPattern = !evaluated.get<BackgroundPattern>().to.empty();

    // TODO: If background is solid, we can skip drawables and rely on the clear color
    const auto drawPasses = evaluated.get<style::BackgroundOpacity>() == 0.0f ? RenderPass::None
                            : (!unevaluated.get<style::BackgroundPattern>().isUndefined() ||
                               evaluated.get<style::BackgroundOpacity>() < 1.0f ||
                               evaluated.get<style::BackgroundColor>().a < 1.0f)
                                ? RenderPass::Translucent
                                : RenderPass::Opaque |
                                      RenderPass::Translucent; // evaluated based on opaquePassCutoff in render()

    // If the result is transparent or missing, just remove any existing drawables and stop
    if (drawPasses == RenderPass::None) {
        removeAllDrawables();
        return;
    }

    if (!layerGroup) {
        if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        } else {
            return;
        }
    }
    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

    if (!layerTweaker) {
        layerTweaker = std::make_shared<BackgroundLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    if (!hasPattern && !plainShader) {
        plainShader = context.getGenericShader(shaders, std::string(BackgroundPlainShaderName));
    }
    if (hasPattern && !patternShader) {
        patternShader = context.getGenericShader(shaders, std::string(BackgroundPatternShaderName));
    }

    const auto& curShader = hasPattern ? patternShader : plainShader;
    if (!curShader) {
        removeAllDrawables();
        return;
    }

    const auto tileVertices = RenderStaticData::tileVertices();
    const auto vertexCount = tileVertices.elements();
    constexpr auto vertSize = sizeof(decltype(tileVertices)::Vertex::a1);
    std::vector<std::uint8_t> rawVertices(vertexCount * vertSize);
    for (auto i = 0ULL; i < vertexCount; ++i) {
        std::memcpy(&rawVertices[i * vertSize], &tileVertices.vector()[i].a1, vertSize);
    }

    const auto indexes = RenderStaticData::quadTriangleIndices();
    const auto segs = RenderStaticData::tileTriangleSegments();

    std::unique_ptr<gfx::DrawableBuilder> builder;

    // Remove drawables for tiles that are no longer in the cover set.
    // (Note that `RenderTiles` is empty, and this layer does not use it)
    tileLayerGroup->removeDrawablesIf([&](gfx::Drawable& drawable) -> bool {
        return drawable.getTileID() && (std::ranges::find(tileCover, *drawable.getTileID()) == tileCover.end());
    });

    // For each tile in the cover set, add a tile drawable if one doesn't already exist.
    for (const auto& tileID : tileCover) {
        // If we already have drawables for this tile, skip.
        // If a drawable needs to be updated, that's handled in the layer tweaker.
        if (tileLayerGroup->getDrawableCount(drawPasses, tileID) > 0) {
            continue;
        }

        // We actually need to build things, so set up a builder if we haven't already
        if (!builder) {
            builder = context.createDrawableBuilder("background");
            builder->setRenderPass(drawPasses);
            builder->setShader(curShader);
            builder->setDepthType(gfx::DepthMaskType::ReadWrite);
            builder->setColorMode(drawPasses == RenderPass::Translucent ? gfx::ColorMode::alphaBlended()
                                                                        : gfx::ColorMode::unblended());
        }

        auto verticesCopy = rawVertices;
        builder->setVertexAttrId(idBackgroundPosVertexAttribute);
        builder->setRawVertices(std::move(verticesCopy), vertexCount, gfx::AttributeDataType::Short2);
        builder->setSegments(gfx::Triangles(), indexes.vector(), segs.data(), segs.size());
        builder->flush(context);

        for (auto& drawable : builder->clearDrawables()) {
            drawable->setTileID(tileID);
            drawable->setLayerTweaker(layerTweaker);
            tileLayerGroup->addDrawable(drawPasses, tileID, std::move(drawable));
            ++stats.drawablesAdded;
        }
    }
}

} // namespace mbgl
