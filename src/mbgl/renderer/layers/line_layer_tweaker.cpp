#include <mbgl/renderer/layers/line_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/line_drawable_data.hpp>
#include <mbgl/renderer/image_atlas.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/math.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/line.hpp>
#endif // MLN_RENDER_BACKEND_METAL

namespace mbgl {

using namespace style;
using namespace shaders;

void LineLayerTweaker::execute(LayerGroupBase& layerGroup,
                               const RenderTree& renderTree,
                               const PaintParameters& parameters) {
    auto& context = parameters.context;
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;
    const auto& crossfade = static_cast<const LineLayerProperties&>(*evaluatedProperties).crossfade;

    const auto zoom = parameters.state.getZoom();

    const auto getLinePropsBuffer = [&]() {
        if (!linePropertiesBuffer) {
            const LinePropertiesUBO linePropertiesUBO{
                /*color =*/evaluated.get<LineColor>().constantOr(LineColor::defaultValue()),
                /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
                /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
                /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
                /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
                /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
                0,
                0,
                0};
            linePropertiesBuffer = context.createUniformBuffer(&linePropertiesUBO, sizeof(linePropertiesUBO));
        }
        return linePropertiesBuffer;
    };
    const auto getLineGradientPropsBuffer = [&]() {
        if (!lineGradientPropertiesBuffer) {
            const LineGradientPropertiesUBO lineGradientPropertiesUBO{
                /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
                /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
                /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
                /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
                /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
                0,
                0,
                0};
            lineGradientPropertiesBuffer = context.createUniformBuffer(&lineGradientPropertiesUBO,
                                                                       sizeof(lineGradientPropertiesUBO));
        }
        return lineGradientPropertiesBuffer;
    };
    const auto getLineSDFPropsBuffer = [&]() {
        if (!lineSDFPropertiesBuffer) {
            const LineSDFPropertiesUBO lineSDFPropertiesUBO{
                /*color =*/evaluated.get<LineColor>().constantOr(LineColor::defaultValue()),
                /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
                /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
                /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
                /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
                /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
                /*floorwidth =*/evaluated.get<LineFloorWidth>().constantOr(LineFloorWidth::defaultValue()),
                0,
                0};
            lineSDFPropertiesBuffer = context.createUniformBuffer(&lineSDFPropertiesUBO, sizeof(lineSDFPropertiesUBO));
        }
        return lineSDFPropertiesBuffer;
    };

#if MLN_RENDER_BACKEND_METAL
    using LineShaderClass = shaders::ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>;
    if (propertiesChanged) {
        const auto source = [this](const std::string_view& attrName) {
            return hasPropertyAsUniform(attrName) ? AttributeSource::Constant : AttributeSource::PerVertex;
        };

        const LinePermutationUBO permutationUBO = {
            /* .color = */ {/*.source=*/source(LineShaderClass::attributes[2].name), /*.expression=*/{}},
            /* .blur = */ {/*.source=*/source(LineShaderClass::attributes[3].name), /*.expression=*/{}},
            /* .opacity = */ {/*.source=*/source(LineShaderClass::attributes[4].name), /*.expression=*/{}},
            /* .gapwidth = */ {/*.source=*/source(LineShaderClass::attributes[5].name), /*.expression=*/{}},
            /* .offset = */ {/*.source=*/source(LineShaderClass::attributes[6].name), /*.expression=*/{}},
            /* .width = */ {/*.source=*/source(LineShaderClass::attributes[7].name), /*.expression=*/{}},
            /* .floorwidth = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
            /* .pattern_from = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
            /* .pattern_to = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
            /* .overdrawInspector = */ overdrawInspector,
            /* .pad = */ 0,
            0,
            0,
            0};

        if (permutationUniformBuffer) {
            permutationUniformBuffer->update(&permutationUBO, sizeof(permutationUBO));
        } else {
            permutationUniformBuffer = context.createUniformBuffer(&permutationUBO, sizeof(permutationUBO));
        }
        propertiesChanged = false;
    }
    if (!expressionUniformBuffer) {
        const ExpressionInputsUBO expressionUBO = {/* .time = */ 0,
                                                   /* .frame = */ parameters.frameCount,
                                                   /* .zoom = */ static_cast<float>(zoom),
                                                   /* .pad = */ 0,
                                                   0,
                                                   0};
        expressionUniformBuffer = context.createUniformBuffer(&expressionUBO, sizeof(expressionUBO));
    }
#endif

    layerGroup.visitDrawables([&](gfx::Drawable& drawable) {
        const auto shader = drawable.getShader();
        if (!drawable.getTileID() || !shader) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto& translation = evaluated.get<LineTranslate>();
        const auto anchor = evaluated.get<LineTranslateAnchor>();
        constexpr bool nearClipped = false;
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        auto& uniforms = drawable.mutableUniformBuffers();
        const auto& shaderUniforms = shader->getUniformBlocks();

        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, nearClipped, inViewportPixelUnits);

        // simple line
        if (shaderUniforms.get(MLN_STRINGIZE(LineUBO))) {
            // main UBO
            const LineUBO lineUBO{
                /*matrix = */ util::cast<float>(matrix),
                /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),
                /*device_pixel_ratio = */ parameters.pixelRatio};
            uniforms.createOrUpdate(MLN_STRINGIZE(LineUBO), &lineUBO, context);

            // properties UBO
            uniforms.addOrReplace(MLN_STRINGIZE(LinePropertiesUBO), getLinePropsBuffer());
        }
        // gradient line
        else if (shaderUniforms.get(MLN_STRINGIZE(LineGradientUBO))) {
            // main UBO
            const LineGradientUBO lineGradientUBO{
                /*matrix = */ util::cast<float>(matrix),
                /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),
                /*device_pixel_ratio = */ parameters.pixelRatio};
            uniforms.createOrUpdate(MLN_STRINGIZE(LineGradientUBO), &lineGradientUBO, context);

            // properties UBO
            uniforms.addOrReplace(MLN_STRINGIZE(LineGradientPropertiesUBO), getLineGradientPropsBuffer());
        }
        // pattern line
        else if (shaderUniforms.get(MLN_STRINGIZE(LinePatternUBO))) {
            // main UBO
            Size textureSize{0, 0};
            if (const auto index = shader->getSamplerLocation("u_image")) {
                if (const auto& texture = drawable.getTexture(index.value())) {
                    textureSize = texture->getSize();
                }
            }
            const LinePatternUBO linePatternUBO{
                /*matrix =*/util::cast<float>(matrix),
                /*scale =*/
                {parameters.pixelRatio,
                 1 / tileID.pixelsToTileUnits(1, parameters.state.getIntegerZoom()),
                 crossfade.fromScale,
                 crossfade.toScale},
                /*texsize =*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                /*units_to_pixels =*/{1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                /*ratio =*/1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),
                /*device_pixel_ratio =*/parameters.pixelRatio,
                /*fade =*/crossfade.t,
                0};
            uniforms.createOrUpdate(MLN_STRINGIZE(LinePatternUBO), &linePatternUBO, context);

            // properties UBO
            const LinePatternPropertiesUBO linePatternPropertiesUBO{
                /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
                /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
                /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
                /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
                /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
                0,
                0,
                0};
            uniforms.createOrUpdate(MLN_STRINGIZE(LinePatternPropertiesUBO), &linePatternPropertiesUBO, context);
        }
        // SDF line
        else if (shaderUniforms.get(MLN_STRINGIZE(LineSDFUBO))) {
            if (const auto& data = drawable.getData()) {
                const gfx::LineDrawableData& lineData = static_cast<const gfx::LineDrawableData&>(*data);
                const auto& dashPatternTexture = parameters.lineAtlas.getDashPatternTexture(
                    evaluated.get<LineDasharray>().from, evaluated.get<LineDasharray>().to, lineData.linePatternCap);

                // texture
                if (const auto index = shader->getSamplerLocation("u_image")) {
                    if (!drawable.getTexture(index.value())) {
                        const auto& texture = dashPatternTexture.getTexture();
                        drawable.setEnabled(!!texture);
                        if (texture) {
                            drawable.setTexture(texture, index.value());
                        }
                    }
                }

                // main UBO
                const LinePatternPos& posA = dashPatternTexture.getFrom();
                const LinePatternPos& posB = dashPatternTexture.getTo();
                const float widthA = posA.width * crossfade.fromScale;
                const float widthB = posB.width * crossfade.toScale;
                const LineSDFUBO lineSDFUBO{
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
                uniforms.createOrUpdate(MLN_STRINGIZE(LineSDFUBO), &lineSDFUBO, context);

                // properties UBO
                uniforms.addOrReplace(MLN_STRINGIZE(LineSDFPropertiesUBO), getLineSDFPropsBuffer());
            }
        }

#if MLN_RENDER_BACKEND_METAL
        uniforms.addOrReplace(MLN_STRINGIZE(ExpressionInputsUBO), expressionUniformBuffer);
        uniforms.addOrReplace(MLN_STRINGIZE(LinePermutationUBO), permutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
    });
}

} // namespace mbgl
