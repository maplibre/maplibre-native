#include <mbgl/style/layers/background_layer_impl.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
// #include <mbgl/shaders/gl/shader_program_gl.hpp>
// #include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/tile_cover.hpp>

#include <unordered_set>

namespace mbgl {
namespace style {

BackgroundLayer::Impl::Impl(const Impl& other)
    : Layer::Impl(other),
      lastColor(other.lastColor),
      paint(other.paint) {}

bool BackgroundLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

constexpr auto shaderName = "background_generic";

void BackgroundLayer::Impl::layerAdded(gfx::ShaderRegistry& shaders,
                                       gfx::Context& context,
                                       const TransformState&,
                                       UniqueChangeRequestVec&) const {
    {
        std::unique_lock<std::mutex> guard(mutex);
        if (!shader) {
            shader = context.getGenericShader(shaders, shaderName);
        }
    }
}

void BackgroundLayer::Impl::layerRemoved(UniqueChangeRequestVec& changes) const {
    // TODO: This isn't happening on style change, so old tile drawables are being left active

    // Remove everything
    decltype(tileDrawables) localDrawables;
    {
        std::unique_lock<std::mutex> guard(mutex);
        localDrawables = std::move(tileDrawables);
    }

    removeDrawables<decltype(localDrawables)::const_iterator>(
        localDrawables.cbegin(), localDrawables.cend(), changes, [](auto& ii) { return ii->second->getId(); });
}

void BackgroundLayer::Impl::update(const int32_t layerIndex,
                                   gfx::Context& context,
                                   const TransformState& state,
                                   UniqueChangeRequestVec& changes) const {
    std::unique_lock<std::mutex> guard(mutex);

    if (!unevaluatedProperties || !evaluatedProperties) {
        // not evaluated yet, we can't update
        return;
    }
    const auto& evaluated = **evaluatedProperties;

    // TODO: If background is solid, we can skip drawables and rely on the clear color
    // const auto passes = eval.get<style::BackgroundOpacity>() == 0.0f
    //    ? RenderPass::None
    //    : (!uneval.get<style::BackgroundPattern>().isUndefined()
    //       || eval.get<style::BackgroundOpacity>() < 1.0f
    //       || eval.get<style::BackgroundColor>().a < 1.0f)
    //    ? RenderPass::Translucent
    //    : RenderPass::Opaque | RenderPass::Translucent;   // evaluated based on opaquePassCutoff in render()

    // unevaluated.hasTransition();
    // getCrossfade<BackgroundLayerProperties>(evaluatedProperties).t != 1;

    std::optional<Color> color;
    if (evaluated.get<BackgroundPattern>().from.empty()) {
        const auto opacity = evaluated.get<style::BackgroundOpacity>();
        if (opacity > 0.0f) {
            color = evaluated.get<BackgroundColor>() * evaluated.get<BackgroundOpacity>();
        }
    }

    // If the result is transparent or missing, just remove any existing drawables and stop
    if (!color) {
        removeDrawables<decltype(tileDrawables)::const_iterator>(
            tileDrawables.cbegin(), tileDrawables.cend(), changes, [](auto& ii) { return ii->second->getId(); });
        tileDrawables.clear();
        return;
    }

    const bool colorChange = (color != lastColor);
    const bool layerChange = (layerIndex != lastLayerIndex);
    lastColor = color;
    lastLayerIndex = layerIndex;

    const auto zoom = state.getIntegerZoom();
    const auto tileCover = util::tileCover(state, zoom);

    // Drawables per overscaled or canonical tile?
    // const UnwrappedTileID unwrappedTileID = tileID.toUnwrapped();

    // Put the tile cover into a searchable form.
    // TODO: Likely better to sort and `std::binary_search` the vector.
    // If it's returned in a well-defined order, we might not even need to sort.
    const std::unordered_set<OverscaledTileID> newTileIDs(tileCover.begin(), tileCover.end());

    // For each existing tile drawable...
    for (auto iter = tileDrawables.begin(); iter != tileDrawables.end();) {
        const auto& drawable = iter->second;

        // Has this tile dropped out of the cover set?
        if (newTileIDs.find(iter->first) == newTileIDs.end()) {
            // remove it
            changes.emplace_back(std::make_unique<RemoveDrawableRequest>(drawable->getId()));
            // Log::Warning(Event::General, "Removing drawable for " + util::toString(iter->first) + " total " +
            // std::to_string(stats.tileDrawablesRemoved+1));
            iter = tileDrawables.erase(iter);
            ++stats.tileDrawablesRemoved;
            continue;
        }
        ++iter;

        // If the color evaluated to a new value, update all vertexes of the drawable to the new color
        if (colorChange) {
            drawable->resetColor(*color);
        }
        if (layerChange) {
            drawable->setLayerIndex(layerIndex);
        }
    }

    std::unique_ptr<gfx::DrawableBuilder> builder;

    // For each tile in the cover set, add a tile drawable if one doesn't already exist.
    // We currently assume only one drawable per tile.
    for (const auto& tileID : tileCover) {
        const auto result = tileDrawables.insert(std::make_pair(tileID, gfx::DrawablePtr()));
        if (!result.second) {
            // Already present
            // TODO: Update matrix here or in the tweaker?
            continue;
        }

        // We actually need to build things, so set up a builder if we haven't already
        if (!builder) {
            builder = context.createDrawableBuilder("background");
            builder->setShader(shader);
            builder->addTweaker(context.createDrawableTweaker());
            builder->setColor(*color);
            builder->setColorMode(gfx::DrawableBuilder::ColorMode::PerDrawable);
            builder->setDepthType(gfx::DepthMaskType::ReadWrite);
            builder->setLayerIndex(layerIndex);
        }

        // Tile coordinates are fixed...
        builder->addQuad(0, 0, util::EXTENT, util::EXTENT);

        // ... they're placed with the matrix in the uniforms, which changes with the view
        builder->setMatrix(/*parameters.matrixForTile(tileID.toUnwrapped())*/ matrix::identity4());

        builder->flush();

        auto drawables = builder->clearDrawables();
        if (!drawables.empty()) {
            auto& drawable = drawables[0];
            drawable->setTileID(tileID);
            result.first->second = drawable;
            changes.emplace_back(std::make_unique<AddDrawableRequest>(std::move(drawable)));
            ++stats.tileDrawablesAdded;
            // Log::Warning(Event::General, "Adding drawable for " + util::toString(tileID) + " total " +
            // std::to_string(stats.tileDrawablesAdded+1));
        }
    }
}

} // namespace style
} // namespace mbgl
