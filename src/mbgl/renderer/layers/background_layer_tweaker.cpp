#include <mbgl/renderer/layers/background_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/pattern_atlas.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/background_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) BackgroundDrawableUBO {
    std::array<float, 4 * 4> matrix;
};
static_assert(sizeof(BackgroundDrawableUBO) % 16 == 0);

struct alignas(16) BackgroundLayerUBO {
    /*  0 */ Color color;
    /* 16 */ float opacity;
    /* 20 */ float pad1, pad2, pad3;
    /* 32 */
};
static_assert(sizeof(BackgroundLayerUBO) == 32);

struct alignas(16) BackgroundPatternLayerUBO {
    /*  0 */ std::array<float, 2> pattern_tl_a;
    /*  8 */ std::array<float, 2> pattern_br_a;
    /* 16 */ std::array<float, 2> pattern_tl_b;
    /* 24 */ std::array<float, 2> pattern_br_b;
    /* 32 */ std::array<float, 2> texsize;
    /* 40 */ std::array<float, 2> pattern_size_a;
    /* 48 */ std::array<float, 2> pattern_size_b;
    /* 56 */ std::array<float, 2> pixel_coord_upper;
    /* 64 */ std::array<float, 2> pixel_coord_lower;
    /* 72 */ float tile_units_to_pixels;
    /* 76 */ float scale_a;
    /* 80 */ float scale_b;
    /* 84 */ float mix;
    /* 88 */ float opacity;
    /* 92 */ float pad;
    /* 96 */
};
static_assert(sizeof(BackgroundPatternLayerUBO) == 96);

#if !defined(NDEBUG)
static constexpr auto BackgroundPatternShaderName = "BackgroundPatternShader";
#endif
static constexpr auto BackgroundDrawableUBOName = "BackgroundDrawableUBO";
static constexpr auto BackgroundLayerUBOName = "BackgroundLayerUBO";
static constexpr auto texUniformName = "u_image";

void BackgroundLayerTweaker::execute(LayerGroupBase& layerGroup, const RenderTree&, const PaintParameters& parameters) {
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

    int32_t samplerLocation = -1;
    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        assert(drawable.getTileID());

        // We assume that drawables don't change between pattern and non-pattern.
        const auto& shader = drawable.getShader();
        assert(hasPattern ==
               (shader == context.getGenericShader(parameters.shaders, std::string(BackgroundPatternShaderName))));

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto matrix = parameters.matrixForTile(tileID);

        const BackgroundDrawableUBO drawableUBO = {/* .matrix = */ util::cast<float>(matrix)};

        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.createOrUpdate(BackgroundDrawableUBOName, &drawableUBO, context);

        if (hasPattern) {
            if (samplerLocation < 0) {
                if (const auto index = shader->getSamplerLocation(texUniformName)) {
                    samplerLocation = *index;
                }
            }
            if (0 <= samplerLocation) {
                if (const auto& tex = parameters.patternAtlas.texture()) {
                    drawable.setTexture(tex, samplerLocation);
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
                /* .pad = */ 0,
            };
            uniforms.createOrUpdate(BackgroundLayerUBOName, &layerUBO, context);
        } else {
            const BackgroundLayerUBO layerUBO = {/* .color = */ evaluated.get<BackgroundColor>(),
                                                 /* .opacity = */ evaluated.get<BackgroundOpacity>(),
                                                 /* .pad = */ 0,
                                                 0,
                                                 0};
            uniforms.createOrUpdate(BackgroundLayerUBOName, &layerUBO, context);
        }
    });
}

} // namespace mbgl
