#include <mbgl/renderer/layers/background_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/pattern_atlas.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/background_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/string_indexer.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

#if !defined(NDEBUG)
constexpr auto BackgroundPatternShaderName = "BackgroundPatternShader";
#endif
static const StringIdentity idBackgroundDrawableUBOName = stringIndexer().get("BackgroundDrawableUBO");
static const StringIdentity idBackgroundLayerUBOName = stringIndexer().get("BackgroundLayerUBO");
static const StringIdentity idTexUniformName = stringIndexer().get("u_image");

void BackgroundLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    const auto& state = parameters.state;
    auto& context = parameters.context;

    if (layerGroup.empty()) {
        return;
    }

#if defined(DEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    const auto& evaluated = static_cast<const BackgroundLayerProperties&>(*evaluatedProperties).evaluated;
    const auto& crossfade = static_cast<const BackgroundLayerProperties&>(*evaluatedProperties).crossfade;
    const bool hasPattern = !evaluated.get<BackgroundPattern>().to.empty();
    const auto imagePosA = hasPattern ? parameters.patternAtlas.getPattern(evaluated.get<BackgroundPattern>().from.id())
                                      : std::nullopt;
    const auto imagePosB = hasPattern ? parameters.patternAtlas.getPattern(evaluated.get<BackgroundPattern>().to.id())
                                      : std::nullopt;

    if (hasPattern && (!imagePosA || !imagePosB)) {
        // The pattern isn't valid, disable the whole thing.
        layerGroup.setEnabled(false);
        return;
    }
    layerGroup.setEnabled(true);

    std::optional<uint32_t> samplerLocation{};
    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        assert(drawable.getTileID());
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        // We assume that drawables don't change between pattern and non-pattern.
        const auto& shader = drawable.getShader();
        assert(hasPattern ==
               (shader == context.getGenericShader(parameters.shaders, std::string(BackgroundPatternShaderName))));

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto matrix = parameters.matrixForTile(tileID);

        const BackgroundDrawableUBO drawableUBO = {/* .matrix = */ util::cast<float>(matrix)};

        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.createOrUpdate(idBackgroundDrawableUBOName, &drawableUBO, context);

        if (hasPattern) {
            if (!samplerLocation.has_value()) {
                samplerLocation = shader->getSamplerLocation(idTexUniformName);
                if (const auto& tex = parameters.patternAtlas.texture()) {
                    tex->setSamplerConfiguration(
                        {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
                }
            }
            if (samplerLocation.has_value()) {
                if (const auto& tex = parameters.patternAtlas.texture()) {
                    drawable.setTexture(tex, samplerLocation.value());
                }
            }

            // from BackgroundPatternProgram::layoutUniformValues
            const int32_t tileSizeAtNearestZoom = static_cast<int32_t>(
                util::tileSize_D * state.zoomScale(state.getIntegerZoom() - tileID.canonical.z));
            const int32_t pixelX = static_cast<int32_t>(
                tileSizeAtNearestZoom * (tileID.canonical.x + tileID.wrap * state.zoomScale(tileID.canonical.z)));
            const int32_t pixelY = tileSizeAtNearestZoom * tileID.canonical.y;
            const Size atlasSize = parameters.patternAtlas.getPixelSize();
            const auto pixToTile = tileID.pixelsToTileUnits(1.0f, state.getIntegerZoom());

            const BackgroundPatternLayerUBO layerUBO = {
                /* .pattern_tl_a = */ util::cast<float>(imagePosA->tl()),
                /* .pattern_br_a = */ util::cast<float>(imagePosA->br()),
                /* .pattern_tl_b = */ util::cast<float>(imagePosB->tl()),
                /* .pattern_br_b = */ util::cast<float>(imagePosB->br()),
                /* .texsize = */ {static_cast<float>(atlasSize.width), static_cast<float>(atlasSize.height)},
                /* .pattern_size_a = */ imagePosA->displaySize(),
                /* .pattern_size_b = */ imagePosB->displaySize(),
                /* .pixel_coord_upper = */ {static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                /* .pixel_coord_lower = */ {static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                /* .tile_units_to_pixels = */ (pixToTile != 0) ? 1.0f / pixToTile : 0.0f,
                /* .scale_a = */ crossfade.fromScale,
                /* .scale_b = */ crossfade.toScale,
                /* .mix = */ crossfade.t,
                /* .opacity = */ evaluated.get<BackgroundOpacity>(),
                /* .pad1 = */ 0,
            };
            uniforms.createOrUpdate(idBackgroundLayerUBOName, &layerUBO, context);
        } else {
            // UBOs can be shared
            if (!backgroundLayerBuffer) {
                const BackgroundLayerUBO layerUBO = {/* .color = */ evaluated.get<BackgroundColor>(),
                                                     /* .opacity = */ evaluated.get<BackgroundOpacity>(),
                                                     /* .pad1/2/3 = */ 0,
                                                     0,
                                                     0};
                backgroundLayerBuffer = context.createUniformBuffer(&layerUBO, sizeof(layerUBO));
            }
            uniforms.addOrReplace(idBackgroundLayerUBOName, backgroundLayerBuffer);
        }
    });
}

} // namespace mbgl
