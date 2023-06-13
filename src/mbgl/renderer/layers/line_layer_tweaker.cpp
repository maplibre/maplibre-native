#include <mbgl/renderer/layers/line_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/math.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) LineUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> units_to_pixels;
    float ratio;
    float device_pixel_ratio;
};
static_assert(sizeof(LineUBO) % 16 == 0);
static constexpr std::string_view LineUBOName = "LineUBO";

struct alignas(16) LinePropertiesUBO {
    Color color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    
    float pad1;
    std::array<float,2> pad2;
};
static_assert(sizeof(LinePropertiesUBO) % 16 == 0);
static constexpr std::string_view LinePropertiesUBOName = "LinePropertiesUBO";

struct alignas(16) LineGradientUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> units_to_pixels;
    float ratio;
    float device_pixel_ratio;
};
static_assert(sizeof(LineGradientUBO) % 16 == 0);
[[maybe_unused]] static constexpr std::string_view LineGradientUBOName = "LineGradientUBO";

struct alignas(16) LineGradientPropertiesUBO {
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    
    float pad1;
    std::array<float,2> pad2;
};
static_assert(sizeof(LineGradientPropertiesUBO) % 16 == 0);
[[maybe_unused]] static constexpr std::string_view LineGradientPropertiesUBOName = "LineGradientPropertiesUBO";

struct alignas(16) LinePatternUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 4> scale;
    std::array<float, 2> texsize;
    std::array<float, 2> units_to_pixels;
    float ratio;
    float device_pixel_ratio;
    float fade;
    
    float pad1;
};
static_assert(sizeof(LinePatternUBO) % 16 == 0);
[[maybe_unused]] static constexpr std::string_view LinePatternUBOName = "LinePatternUBO";

struct alignas(16) LinePatternPropertiesUBO {
    std::array<float, 4> pattern_from;
    std::array<float, 4> pattern_to;
    float blur;
    float opacity;
    float offset;
    float gapwidth;
    float width;

    float pad1;
    std::array<float,2> pad2;
};
static_assert(sizeof(LinePatternPropertiesUBO) % 16 == 0);
[[maybe_unused]] static constexpr std::string_view LinePatternPropertiesUBOName = "LinePatternPropertiesUBO";

struct alignas(16) LineSDFUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> units_to_pixels;
    std::array<float, 2> patternscale_a;
    std::array<float, 2> patternscale_b;
    float ratio;
    float device_pixel_ratio;
    float tex_y_a;
    float tex_y_b;
    float sdfgamma;
    float mix;
};
static_assert(sizeof(LineSDFUBO) % 16 == 0);
[[maybe_unused]] static constexpr std::string_view LineSDFUBOName = "LineSDFUBO";

struct alignas(16) LineSDFPropertiesUBO {
    Color color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    
    std::array<float,2> pad1;
};
static_assert(sizeof(LineSDFPropertiesUBO) % 16 == 0);
[[maybe_unused]] static constexpr std::string_view LineSDFPropertiesUBOName = "LineSDFPropertiesUBO";

void LineLayerTweaker::execute(LayerGroupBase& layerGroup,
                               const RenderTree& renderTree,
                               const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;

    LinePropertiesUBO linePropertiesUBO {
        /*color =*/ evaluated.get<LineColor>().constantOr(LineColor::defaultValue()),
        /*blur =*/ evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
        /*opacity =*/ evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
        /*gapwidth =*/ evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
        /*offset =*/ evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
        /*width =*/ evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
        0, {0, 0}
    };
    evaluatedPropsUniformBuffer = parameters.context.createUniformBuffer(&linePropertiesUBO, sizeof(linePropertiesUBO));

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        drawable.mutableUniformBuffers().addOrReplace(LinePropertiesUBOName, evaluatedPropsUniformBuffer);

        if (!drawable.getTileID()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto& translation = evaluated.get<LineTranslate>();
        const auto anchor = evaluated.get<LineTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, inViewportPixelUnits);

        LineUBO lineUBO;
        lineUBO.matrix = util::cast<float>(matrix);
        lineUBO.ratio = 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom()));
        lineUBO.units_to_pixels = {{1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]}};
        lineUBO.device_pixel_ratio = parameters.pixelRatio;
        auto drawableUniformBuffer = parameters.context.createUniformBuffer(&lineUBO, sizeof(lineUBO));
        drawable.mutableUniformBuffers().addOrReplace(LineUBOName, drawableUniformBuffer);
    });
}

} // namespace mbgl
