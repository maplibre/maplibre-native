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

struct alignas(16) LineDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> units_to_pixels;
    float ratio;
    float device_pixel_ratio;
};
static_assert(sizeof(LineDrawableUBO) == 80);
static_assert(sizeof(LineDrawableUBO) % 16 == 0);

struct alignas(16) LineEvaluatedPropsUBO {
    Color color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float pad1;
    float pad2;
    float pad3;
};
static_assert(sizeof(LineEvaluatedPropsUBO) == 48);
static_assert(sizeof(LineEvaluatedPropsUBO) % 16 == 0);

void LineLayerTweaker::execute(LayerGroupBase& layerGroup,
                               const RenderTree& renderTree,
                               const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;

    LineEvaluatedPropsUBO evaluatedPropsUBO;
    evaluatedPropsUBO.color = evaluated.get<LineColor>().constantOr(Color(1, 0, 1, 1));
    evaluatedPropsUBO.blur = evaluated.get<LineBlur>().constantOr(0);
    evaluatedPropsUBO.opacity = evaluated.get<LineOpacity>().constantOr(0);
    evaluatedPropsUBO.gapwidth = evaluated.get<LineGapWidth>().constantOr(0);
    evaluatedPropsUBO.offset = evaluated.get<LineOffset>().constantOr(0);
    evaluatedPropsUBO.width = evaluated.get<LineWidth>().constantOr(0);
    evaluatedPropsUniformBuffer = parameters.context.createUniformBuffer(&evaluatedPropsUBO, sizeof(evaluatedPropsUBO));

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        drawable.mutableUniformBuffers().addOrReplace("LineEvaluatedPropsUBO", evaluatedPropsUniformBuffer);

        if (!drawable.getTileID()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto& translation = evaluated.get<LineTranslate>();
        const auto anchor = evaluated.get<LineTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, inViewportPixelUnits);

        LineDrawableUBO drawableUBO;
        drawableUBO.matrix = util::cast<float>(matrix);
        drawableUBO.ratio = 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom()));
        drawableUBO.units_to_pixels = {{1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]}};
        drawableUBO.device_pixel_ratio = parameters.pixelRatio;
        auto drawableUniformBuffer = parameters.context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
        drawable.mutableUniformBuffers().addOrReplace("LineDrawableUBO", drawableUniformBuffer);
    });
}

} // namespace mbgl
