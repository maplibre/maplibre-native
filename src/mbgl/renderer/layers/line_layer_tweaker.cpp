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
    std::array<float, 3> pad;
};
static_assert(sizeof(LineEvaluatedPropsUBO) == 48);
static_assert(sizeof(LineEvaluatedPropsUBO) % 16 == 0);

struct alignas(16) LineInterpolatedPropsUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    std::array<float, 2> pad;
};
static_assert(sizeof(LineInterpolatedPropsUBO) == 32);
static_assert(sizeof(LineInterpolatedPropsUBO) % 16 == 0);

void LineLayerTweaker::execute(LayerGroup& layerGroup, const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;

    if (!evaluatedPropsUniformBuffer) {
        LineEvaluatedPropsUBO evaluatedPropsUBO;
        evaluatedPropsUBO.color = Color(1, 0, 1, 1); // evaluated.get<LineColor>().constantOr(Color(1, 0, 1, 1));
        evaluatedPropsUBO.blur = evaluated.get<LineBlur>().constantOr(0);
        evaluatedPropsUBO.opacity = evaluated.get<LineOpacity>().constantOr(0);
        evaluatedPropsUBO.gapwidth = evaluated.get<LineGapWidth>().constantOr(0);
        evaluatedPropsUBO.offset = evaluated.get<LineOffset>().constantOr(0);
        evaluatedPropsUBO.width = evaluated.get<LineWidth>().constantOr(0);
        evaluatedPropsUniformBuffer = parameters.context.createUniformBuffer(&evaluatedPropsUBO,
                                                                             sizeof(evaluatedPropsUBO));
    }

    LineInterpolatedPropsUBO interpolatedUBO;
    interpolatedUBO.color_t = 0;
    interpolatedUBO.blur_t = 0;
    interpolatedUBO.opacity_t = 0;
    interpolatedUBO.gapwidth_t = 0;
    interpolatedUBO.offset_t = 0;
    interpolatedUBO.width_t = 0;
    auto interpolateUniformBuffer = parameters.context.createUniformBuffer(&interpolatedUBO, sizeof(interpolatedUBO));

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        drawable.mutableUniformBuffers().addOrReplace("LineEvaluatedPropsUBO", evaluatedPropsUniformBuffer);
        drawable.mutableUniformBuffers().addOrReplace("LineInterpolatedUBO", interpolateUniformBuffer);

        if (!drawable.getTileID()) {
            return;
        }

        // TODO: refactor this, use RenderTile::translatedMatrix?
        auto translateVtxMatrix = [](const UnwrappedTileID& id,
                                     const mat4& tileMatrix,
                                     const std::array<float, 2>& translation,
                                     TranslateAnchorType anchor,
                                     const TransformState& state,
                                     const bool inViewportPixelUnits) -> mat4 {
            if (translation[0] == 0 && translation[1] == 0) {
                return tileMatrix;
            }

            mat4 vtxMatrix;

            const float angle =
                inViewportPixelUnits
                    ? (anchor == TranslateAnchorType::Map ? static_cast<float>(state.getBearing()) : 0.0f)
                    : (anchor == TranslateAnchorType::Viewport ? static_cast<float>(-state.getBearing()) : 0.0f);

            Point<float> translate = util::rotate(Point<float>{translation[0], translation[1]}, angle);

            if (inViewportPixelUnits) {
                matrix::translate(vtxMatrix, tileMatrix, translate.x, translate.y, 0);
            } else {
                matrix::translate(vtxMatrix,
                                  tileMatrix,
                                  id.pixelsToTileUnits(translate.x, static_cast<float>(state.getZoom())),
                                  id.pixelsToTileUnits(translate.y, static_cast<float>(state.getZoom())),
                                  0);
            }

            return vtxMatrix;
        };

        /* mat4 matrix = renderTile.translatedMatrix(properties.get<LineTranslate>(),
         * properties.get<LineTranslateAnchor>(), state) */
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        mat4 matrix;
        parameters.state.matrixFor(matrix, tileID);
        matrix = translateVtxMatrix(tileID,
                                    matrix,
                                    evaluated.get<LineTranslate>(),
                                    evaluated.get<LineTranslateAnchor>(),
                                    parameters.state,
                                    false);
        matrix::multiply(matrix, parameters.transformParams.projMatrix, matrix);

        //        mat4 matrix = drawable.getMatrix();
        //        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        //        const auto tileMat = parameters.matrixForTile(tileID);
        //        matrix::multiply(matrix, drawable.getMatrix(), tileMat);
        //        matrix = tileMat;

        //        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        //        mat4 matrix = parameters.matrixForTile(tileID);

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
