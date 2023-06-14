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
    float blur;
    float opacity;
    float offset;
    float gapwidth;
    float width;

    float pad1;
    std::array<float,2> pad2;
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
    const auto& crossfade = static_cast<const LineLayerProperties&>(*evaluatedProperties).crossfade;

    LinePropertiesUBO linePropertiesUBO {
        /*color =*/ evaluated.get<LineColor>().constantOr(LineColor::defaultValue()),
        /*blur =*/ evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
        /*opacity =*/ evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
        /*gapwidth =*/ evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
        /*offset =*/ evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
        /*width =*/ evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
        0, {0, 0}
    };

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
            LineUBO lineUBO;
            lineUBO.matrix = util::cast<float>(matrix);
            lineUBO.ratio = 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom()));
            lineUBO.units_to_pixels = {{1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]}};
            lineUBO.device_pixel_ratio = parameters.pixelRatio;
            auto drawableUniformBuffer = parameters.context.createUniformBuffer(&lineUBO, sizeof(lineUBO));
            drawable.mutableUniformBuffers().addOrReplace(LineUBOName, drawableUniformBuffer);
            
            // properties UBO
            drawable.mutableUniformBuffers().createOrUpdate(LinePropertiesUBOName, &linePropertiesUBO, sizeof(linePropertiesUBO), parameters.context);
        }
        // gradient line
        else if (drawable.getShader()->getUniformBlocks().get(std::string(LineGradientUBOName))) {
            // TODO: main UBO
            // TODO: properties UBO
        }
        // pattern line
        else if (drawable.getShader()->getUniformBlocks().get(std::string(LinePatternUBOName))) {
            // main UBO
            Size textureSize{0, 0};
            if (const auto shader = drawable.getShader()) {
                if (const auto index = shader->getSamplerLocation("u_image")) {
                    if(auto itextureAttachment = std::find_if(drawable.getTextures().begin(), drawable.getTextures().end(), [&index](const auto& textureAttachment) -> bool { return (textureAttachment.texture) && (textureAttachment.location == static_cast<int32_t>(index.value())); });
                       drawable.getTextures().end() != itextureAttachment) {
                        textureSize = itextureAttachment->texture->getSize();
                    }
                }
            }
            LinePatternUBO linePatternUBO {
                /*matrix =*/ util::cast<float>(matrix),
                /*scale =*/ {parameters.pixelRatio,
                    1 / tileID.pixelsToTileUnits(1, parameters.state.getIntegerZoom()),
                    crossfade.fromScale,
                    crossfade.toScale},
                /*texsize =*/ {static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                /*units_to_pixels =*/ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                /*ratio =*/ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom())),
                /*device_pixel_ratio =*/ parameters.pixelRatio,
                /*fade =*/ crossfade.t,
                0
            };
            drawable.mutableUniformBuffers().createOrUpdate(LinePatternUBOName, &linePatternUBO, sizeof(linePatternUBO), parameters.context);

            // properties UBO
            LinePatternPropertiesUBO linePatternPropertiesUBO {
                /*blur =*/ evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
                /*opacity =*/ evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
                /*offset =*/ evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
                /*gapwidth =*/ evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
                /*width =*/ evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
                0, {0, 0}
            };
            drawable.mutableUniformBuffers().createOrUpdate(LinePatternPropertiesUBOName, &linePatternPropertiesUBO, sizeof(linePatternPropertiesUBO), parameters.context);
        }
        // SDF line
        else if (drawable.getShader()->getUniformBlocks().get(std::string(LineSDFUBOName))) {
            // TODO: main UBO
            // TODO: properties UBO
        }
    });
}

} // namespace mbgl
