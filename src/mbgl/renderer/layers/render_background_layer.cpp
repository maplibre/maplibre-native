#include <mbgl/renderer/layers/render_background_layer.hpp>
#include <mbgl/renderer/layers/background_layer_tweaker.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/pattern_atlas.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/upload_parameters.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/background_layer_impl.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/util/tile_cover.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>

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
    if (tileLayerGroup) {
        tileLayerGroup->setLayerTweaker(std::make_shared<BackgroundLayerTweaker>(evaluatedProperties));
    }
}

bool RenderBackgroundLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderBackgroundLayer::hasCrossfade() const {
    return getCrossfade<BackgroundLayerProperties>(evaluatedProperties).t != 1;
}

void RenderBackgroundLayer::render(PaintParameters& parameters) {
    // Note that for bottommost layers without a pattern, the background color
    // is drawn with glClear rather than this method.

    // Ensure programs are available
    if (!parameters.shaders.populate(backgroundProgram)) return;
    if (!parameters.shaders.populate(backgroundPatternProgram)) return;

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

void RenderBackgroundLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    // Remove everything
    if (tileLayerGroup) {
        changes.emplace_back(std::make_unique<RemoveLayerGroupRequest>(tileLayerGroup->getLayerIndex()));
        tileLayerGroup.reset();
    }
}

void RenderBackgroundLayer::update(gfx::ShaderRegistry& shaders,
                                   gfx::Context& context,
                                   const TransformState& state,
                                   [[maybe_unused]] const RenderTree& renderTree,
                                   [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    if (!shader) {
        shader = context.getGenericShader(shaders, "BackgroundShader");
    }

    const auto removeAll = [&]() {
        if (tileLayerGroup) {
            stats.tileDrawablesRemoved += tileLayerGroup->getDrawableCount();
            tileLayerGroup->clearDrawables();
        }
    };

    const auto zoom = state.getIntegerZoom();
    const auto tileCover = util::tileCover(state, zoom);

    // renderTiles is always empty, we use tileCover instead
    // if (!renderTiles || renderTiles->empty()) {
    if (tileCover.empty()) {
        removeAll();
        return;
    }

    const auto& evaluated = getEvaluated<BackgroundLayerProperties>(evaluatedProperties);

    // TODO: If background is solid, we can skip drawables and rely on the clear color
    const auto drawPasses = evaluated.get<style::BackgroundOpacity>() == 0.0f ? RenderPass::None
                            : (!unevaluated.get<style::BackgroundPattern>().isUndefined() ||
                               evaluated.get<style::BackgroundOpacity>() < 1.0f ||
                               evaluated.get<style::BackgroundColor>().a < 1.0f)
                                ? RenderPass::Translucent
                                : RenderPass::Opaque |
                                      RenderPass::Translucent; // evaluated based on opaquePassCutoff in render()

    // unevaluated.hasTransition();
    // getCrossfade<BackgroundLayerProperties>(evaluatedProperties).t != 1;

    // If the result is transparent or missing, just remove any existing drawables and stop
    if (drawPasses == RenderPass::None) {
        removeAll();
        return;
    }

    if (!tileLayerGroup) {
        tileLayerGroup = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64);
        if (!tileLayerGroup) {
            return;
        }
        tileLayerGroup->setLayerTweaker(std::make_shared<BackgroundLayerTweaker>(evaluatedProperties));
    }

    // Drawables per overscaled or canonical tile?
    // const UnwrappedTileID unwrappedTileID = tileID.toUnwrapped();

    // Put the tile cover into a searchable form.
    // TODO: Likely better to sort and `std::binary_search` the vector.
    // If it's returned in a well-defined order, we might not even need to sort.
    const std::unordered_set<OverscaledTileID> newTileIDs(tileCover.begin(), tileCover.end());

    std::unique_ptr<gfx::DrawableBuilder> builder;

    for (const auto renderPass : {RenderPass::Opaque, RenderPass::Translucent}) {
        tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
            // Has this tile dropped out of the cover set?
            const auto tileID = drawable->getTileID();
            if (tileID && newTileIDs.find(*tileID) == newTileIDs.end()) {
                drawable.reset();
                ++stats.tileDrawablesRemoved;
                return;
            }
        });

        // For each tile in the cover set, add a tile drawable if one doesn't already exist.
        // We currently assume only one drawable per tile.
        for (const auto& tileID : tileCover) {
            auto& tileDrawable = tileLayerGroup->getDrawable(renderPass, tileID);

            if (!(renderPass & drawPasses)) {
                // Not in this pass
                if (tileDrawable) {
                    tileLayerGroup->removeDrawables(renderPass, tileID);
                    ++stats.tileDrawablesRemoved;
                }
                continue;
            }

            if (tileDrawable) {
                // Already created, update it.
                continue;
            }

            // We actually need to build things, so set up a builder if we haven't already
            if (!builder) {
                builder = context.createDrawableBuilder("background");
                builder->setRenderPass(drawPasses);
                builder->setShader(shader);
                builder->setColorAttrMode(gfx::DrawableBuilder::ColorAttrMode::PerDrawable);
                builder->setDepthType(gfx::DepthMaskType::ReadWrite);
                builder->setLayerIndex(layerIndex);
            }

            // Tile coordinates are fixed...
            builder->addQuad(0, 0, util::EXTENT, util::EXTENT);

            // ... they're placed with the matrix in the uniforms, which changes with the view
            builder->setMatrix(/*parameters.matrixForTile(tileID.toUnwrapped())*/ matrix::identity4());

            builder->flush();

            auto newDrawables = builder->clearDrawables();
            if (!newDrawables.empty()) {
                auto& drawable = newDrawables[0];
                drawable->setTileID(tileID);
                tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.tileDrawablesAdded;
            }
        }
    }
}

} // namespace mbgl
