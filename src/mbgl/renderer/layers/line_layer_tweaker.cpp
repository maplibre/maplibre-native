#include <mbgl/renderer/layers/line_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/renderer/image_atlas.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/geometry/line_atlas.hpp>
#include <mbgl/gfx/line_drawable_data.hpp>

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
    std::array<float, 2> pad2;
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
static constexpr std::string_view LineGradientUBOName = "LineGradientUBO";

struct alignas(16) LineGradientPropertiesUBO {
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;

    float pad1;
    std::array<float, 2> pad2;
};
static_assert(sizeof(LineGradientPropertiesUBO) % 16 == 0);
static constexpr std::string_view LineGradientPropertiesUBOName = "LineGradientPropertiesUBO";

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
static constexpr std::string_view LinePatternUBOName = "LinePatternUBO";

struct alignas(16) LinePatternPropertiesUBO {
    float blur;
    float opacity;
    float offset;
    float gapwidth;
    float width;

    float pad1;
    std::array<float, 2> pad2;
};
static_assert(sizeof(LinePatternPropertiesUBO) % 16 == 0);
static constexpr std::string_view LinePatternPropertiesUBOName = "LinePatternPropertiesUBO";

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
static constexpr std::string_view LineSDFUBOName = "LineSDFUBO";

struct alignas(16) LineSDFPropertiesUBO {
    Color color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;

    std::array<float, 2> pad1;
};
static_assert(sizeof(LineSDFPropertiesUBO) % 16 == 0);
static constexpr std::string_view LineSDFPropertiesUBOName = "LineSDFPropertiesUBO";

void LineLayerTweaker::execute(LayerGroupBase& layerGroup,
                               const RenderTree& renderTree,
                               const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;
    const auto& crossfade = static_cast<const LineLayerProperties&>(*evaluatedProperties).crossfade;

    LinePropertiesUBO linePropertiesUBO{
        /*color =*/evaluated.get<LineColor>().constantOr(LineColor::defaultValue()),
        /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
        /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
        /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
        /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
        /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
        0,
        {0, 0}};
    LineGradientPropertiesUBO lineGradientPropertiesUBO{
        /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
        /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
        /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
        /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
        /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
        0,
        {0, 0}};
    LineSDFPropertiesUBO lineSDFPropertiesUBO{
        /*color =*/evaluated.get<LineColor>().constantOr(LineColor::defaultValue()),
        /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
        /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
        /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
        /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
        /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
        /*floorwidth =*/evaluated.get<LineFloorWidth>().constantOr(LineFloorWidth::defaultValue()),
        {0, 0}};

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getTileID()) return;

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto& translation = evaluated.get<LineTranslate>();
        const auto anchor = evaluated.get<LineTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, inViewportPixelUnits);

        // simple line
        if (drawable.getShader()->getUniformBlocks().get(std::string(LineUBOName))) {
            // main UBO
            LineUBO lineUBO{
                /*matrix = */ util::cast<float>(matrix),
                /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom())),
                /*device_pixel_ratio = */ parameters.pixelRatio};
            drawable.mutableUniformBuffers().createOrUpdate(LineUBOName, &lineUBO, sizeof(lineUBO), parameters.context);

            // properties UBO
            drawable.mutableUniformBuffers().createOrUpdate(
                LinePropertiesUBOName, &linePropertiesUBO, sizeof(linePropertiesUBO), parameters.context);
        }
        // gradient line
        else if (drawable.getShader()->getUniformBlocks().get(std::string(LineGradientUBOName))) {
            // main UBO
            LineGradientUBO lineGradientUBO{
                /*matrix = */ util::cast<float>(matrix),
                /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom())),
                /*device_pixel_ratio = */ parameters.pixelRatio};
            drawable.mutableUniformBuffers().createOrUpdate(
                LineGradientUBOName, &lineGradientUBO, sizeof(lineGradientUBO), parameters.context);

            // properties UBO
            drawable.mutableUniformBuffers().createOrUpdate(LineGradientPropertiesUBOName,
                                                            &lineGradientPropertiesUBO,
                                                            sizeof(lineGradientPropertiesUBO),
                                                            parameters.context);
        }
        // pattern line
        else if (drawable.getShader()->getUniformBlocks().get(std::string(LinePatternUBOName))) {
            // main UBO
            Size textureSize{0, 0};
            if (const auto shader = drawable.getShader()) {
                if (const auto index = shader->getSamplerLocation("u_image")) {
                    if (const auto& texture = drawable.getTexture(index.value())) {
                        textureSize = texture->getSize();
                    }
                }
            }
            LinePatternUBO linePatternUBO{
                /*matrix =*/util::cast<float>(matrix),
                /*scale =*/
                {parameters.pixelRatio,
                 1 / tileID.pixelsToTileUnits(1, parameters.state.getIntegerZoom()),
                 crossfade.fromScale,
                 crossfade.toScale},
                /*texsize =*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                /*units_to_pixels =*/{1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                /*ratio =*/1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom())),
                /*device_pixel_ratio =*/parameters.pixelRatio,
                /*fade =*/crossfade.t,
                0};
            drawable.mutableUniformBuffers().createOrUpdate(
                LinePatternUBOName, &linePatternUBO, sizeof(linePatternUBO), parameters.context);

            // properties UBO
            LinePatternPropertiesUBO linePatternPropertiesUBO{
                /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
                /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
                /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
                /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
                /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
                0,
                {0, 0}};
            drawable.mutableUniformBuffers().createOrUpdate(LinePatternPropertiesUBOName,
                                                            &linePatternPropertiesUBO,
                                                            sizeof(linePatternPropertiesUBO),
                                                            parameters.context);
        }
        // SDF line
        else if (drawable.getShader()->getUniformBlocks().get(std::string(LineSDFUBOName))) {
            if (const auto& data = drawable.getData()) {
                const gfx::LineDrawableData& lineData = static_cast<const gfx::LineDrawableData&>(*data.value());
                const auto& dashPatternTexture = parameters.lineAtlas.getDashPatternTexture(
                    evaluated.get<LineDasharray>().from, evaluated.get<LineDasharray>().to, lineData.linePatternCap);

                // texture
                if (const auto shader = drawable.getShader()) {
                    if (const auto index = shader->getSamplerLocation("u_image")) {
                        if (!drawable.getTexture(index.value())) {
                            const auto& texture = dashPatternTexture.getTexture();
                            drawable.setEnabled(!!texture);
                            if (texture) {
                                drawable.setTexture(texture, index.value());
                            }
                        }
                    }
                }

                // main UBO
                const LinePatternPos& posA = dashPatternTexture.getFrom();
                const LinePatternPos& posB = dashPatternTexture.getTo();
                const float widthA = posA.width * crossfade.fromScale;
                const float widthB = posB.width * crossfade.toScale;
                LineSDFUBO lineSDFUBO{
                    /* matrix = */ util::cast<float>(matrix),
                    /* units_to_pixels = */
                    {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                    /* patternscale_a = */
                    {1.0f / tileID.pixelsToTileUnits(widthA, parameters.state.getIntegerZoom()), -posA.height / 2.0f},
                    /* patternscale_b = */
                    {1.0f / tileID.pixelsToTileUnits(widthB, parameters.state.getIntegerZoom()), -posB.height / 2.0f},
                    /* ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom())),
                    /* device_pixel_ratio = */ parameters.pixelRatio,
                    /* tex_y_a = */ posA.y,
                    /* tex_y_b = */ posB.y,
                    /* sdfgamma = */ static_cast<float>(dashPatternTexture.getSize().width) /
                        (std::min(widthA, widthB) * 256.0f * parameters.pixelRatio) / 2.0f,
                    /* mix = */ crossfade.t};
                drawable.mutableUniformBuffers().createOrUpdate(
                    LineSDFUBOName, &lineSDFUBO, sizeof(lineSDFUBO), parameters.context);

                // properties UBO
                drawable.mutableUniformBuffers().createOrUpdate(
                    LineSDFPropertiesUBOName, &lineSDFPropertiesUBO, sizeof(lineSDFPropertiesUBO), parameters.context);
            }
        }
    });
}

} // namespace mbgl
