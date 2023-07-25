#include <mbgl/renderer/layers/render_background_layer.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/bucket.hpp>
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

#if MLN_DRAWABLE_RENDERER
#include <mbgl/renderer/layers/background_layer_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#endif

#include <unordered_set>

namespace mbgl {

using namespace style;

namespace {

inline const BackgroundLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == BackgroundLayer::Impl::staticTypeInfo());
    return static_cast<const style::BackgroundLayer::Impl&>(*impl);
}

} // namespace

RenderBackgroundLayer::RenderBackgroundLayer(Immutable<style::BackgroundLayer::Impl> _impl)
    : RenderLayer(makeMutable<BackgroundLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

RenderBackgroundLayer::~RenderBackgroundLayer() = default;

void RenderBackgroundLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
}

void RenderBackgroundLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto evaluated = unevaluated.evaluate(parameters);
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

#if MLN_DRAWABLE_RENDERER
    if (layerGroup) {
        layerGroup->setLayerTweaker(std::make_shared<BackgroundLayerTweaker>(evaluatedProperties));
    }
#endif
}

bool RenderBackgroundLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderBackgroundLayer::hasCrossfade() const {
    return getCrossfade<BackgroundLayerProperties>(evaluatedProperties).t != 1;
}

#if MLN_LEGACY_RENDERER
void RenderBackgroundLayer::render(PaintParameters& parameters) {
    // Note that for bottommost layers without a pattern, the background color
    // is drawn with glClear rather than this method.

    // Ensure programs are available
    if (!parameters.shaders.getLegacyGroup().populate(backgroundProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(backgroundPatternProgram)) return;

    const Properties<>::PossiblyEvaluated properties;
    const BackgroundProgram::Binders paintAttributeData(properties, 0);

    auto draw = [&](auto& program, auto&& uniformValues, const auto& textureBindings, const uint32_t id) {
        const auto allUniformValues = program.computeAllUniformValues(
            std::forward<decltype(uniformValues)>(uniformValues),
            paintAttributeData,
            properties,
            static_cast<float>(parameters.state.getZoom()));
        const auto allAttributeBindings = program.computeAllAttributeBindings(
            *parameters.staticData.tileVertexBuffer, paintAttributeData, properties);

        checkRenderability(parameters, program.activeBindingCount(allAttributeBindings));

        program.draw(
            parameters.context,
            *parameters.renderPass,
            gfx::Triangles(),
            parameters.depthModeForSublayer(
                0,
                parameters.pass == RenderPass::Opaque ? gfx::DepthMaskType::ReadWrite : gfx::DepthMaskType::ReadOnly),
            gfx::StencilMode::disabled(),
            parameters.colorModeForRenderPass(),
            gfx::CullFaceMode::disabled(),
            *parameters.staticData.quadTriangleIndexBuffer,
            segments,
            allUniformValues,
            allAttributeBindings,
            textureBindings,
            util::toString(id));
    };

    if (segments.empty()) {
        segments = RenderStaticData::tileTriangleSegments();
    }

    const auto& evaluated = static_cast<const BackgroundLayerProperties&>(*evaluatedProperties).evaluated;
    const auto& crossfade = static_cast<const BackgroundLayerProperties&>(*evaluatedProperties).crossfade;
    if (!evaluated.get<BackgroundPattern>().to.empty()) {
        std::optional<ImagePosition> imagePosA = parameters.patternAtlas.getPattern(
            evaluated.get<BackgroundPattern>().from.id());
        std::optional<ImagePosition> imagePosB = parameters.patternAtlas.getPattern(
            evaluated.get<BackgroundPattern>().to.id());

        if (!imagePosA || !imagePosB) return;

        uint32_t i = 0;
        for (const auto& tileID : util::tileCover(parameters.state, parameters.state.getIntegerZoom())) {
            const UnwrappedTileID unwrappedTileID = tileID.toUnwrapped();
            draw(*backgroundPatternProgram,
                 BackgroundPatternProgram::layoutUniformValues(parameters.matrixForTile(unwrappedTileID),
                                                               evaluated.get<BackgroundOpacity>(),
                                                               parameters.patternAtlas.getPixelSize(),
                                                               *imagePosA,
                                                               *imagePosB,
                                                               crossfade,
                                                               unwrappedTileID,
                                                               parameters.state),
                 BackgroundPatternProgram::TextureBindings{
                     textures::image::Value{parameters.patternAtlas.textureBinding()},
                 },
                 i++);
        }
    } else {
        auto backgroundRenderPass = (evaluated.get<BackgroundColor>().a >= 1.0f &&
                                     evaluated.get<BackgroundOpacity>() >= 1.0f &&
                                     parameters.currentLayer >= parameters.opaquePassCutoff)
                                        ? RenderPass::Opaque
                                        : RenderPass::Translucent;
        if (parameters.pass != backgroundRenderPass) {
            return;
        }
        uint32_t i = 0;
        for (const auto& tileID : util::tileCover(parameters.state, parameters.state.getIntegerZoom())) {
            draw(*backgroundProgram,
                 BackgroundProgram::LayoutUniformValues{
                     uniforms::matrix::Value(parameters.matrixForTile(tileID.toUnwrapped())),
                     uniforms::color::Value(evaluated.get<BackgroundColor>()),
                     uniforms::opacity::Value(evaluated.get<BackgroundOpacity>()),
                 },
                 BackgroundProgram::TextureBindings{},
                 i++);
        }
    }
}
#endif // MLN_LEGACY_RENDERER

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

#if MLN_DRAWABLE_RENDERER
    updateRenderTileIDs();
#endif // MLN_DRAWABLE_RENDERER
}

#if MLN_DRAWABLE_RENDERER
static constexpr std::string_view BackgroundPlainShaderName = "BackgroundShader";
static constexpr std::string_view BackgroundPatternShaderName = "BackgroundPatternShader";

void RenderBackgroundLayer::update(gfx::ShaderRegistry& shaders,
                                   gfx::Context& context,
                                   const TransformState& state,
                                   [[maybe_unused]] const RenderTree& renderTree,
                                   [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    const auto zoom = state.getIntegerZoom();
    const auto tileCover = util::tileCover(state, zoom);

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
            layerGroup_->setLayerTweaker(std::make_shared<BackgroundLayerTweaker>(evaluatedProperties));
            setLayerGroup(std::move(layerGroup_), changes);
        }
    }
    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

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

    tileLayerGroup->visitDrawables([&](gfx::Drawable& drawable) -> bool {
        // Has this tile dropped out of the cover set?
        return (!drawable.getTileID() || hasRenderTile(*drawable.getTileID()));
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
        builder->setRawVertices(std::move(verticesCopy), vertexCount, gfx::AttributeDataType::Short2);
        builder->setSegments(gfx::Triangles(), indexes.vector(), segs.data(), segs.size());
        builder->flush();

        for (auto& drawable : builder->clearDrawables()) {
            drawable->setTileID(tileID);
            tileLayerGroup->addDrawable(drawPasses, tileID, std::move(drawable));
            ++stats.drawablesAdded;
        }
    }
}
#endif

} // namespace mbgl
