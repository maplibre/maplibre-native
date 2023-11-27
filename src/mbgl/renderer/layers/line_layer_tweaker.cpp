#include <mbgl/renderer/layers/line_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/line_drawable_data.hpp>
#include <mbgl/util/string_indexer.hpp>
#include <mbgl/geometry/line_atlas.hpp>
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
#include <mbgl/shaders/mtl/line_gradient.hpp>
#endif // MLN_RENDER_BACKEND_METAL

namespace mbgl {

using namespace style;
using namespace shaders;

static const StringIdentity idLineUBOName = stringIndexer().get("LineUBO");
static const StringIdentity idLinePropertiesUBOName = stringIndexer().get("LinePropertiesUBO");
static const StringIdentity idLineGradientUBOName = stringIndexer().get("LineGradientUBO");
static const StringIdentity idLineGradientPropertiesUBOName = stringIndexer().get("LineGradientPropertiesUBO");
static const StringIdentity idLinePatternUBOName = stringIndexer().get("LinePatternUBO");
static const StringIdentity idLinePatternPropertiesUBOName = stringIndexer().get("LinePatternPropertiesUBO");
static const StringIdentity idLineSDFUBOName = stringIndexer().get("LineSDFUBO");
static const StringIdentity idLineSDFPropertiesUBOName = stringIndexer().get("LineSDFPropertiesUBO");
static const StringIdentity idTexImageName = stringIndexer().get("u_image");

static const StringIdentity idExpressionInputsUBOName = stringIndexer().get("ExpressionInputsUBO");
static const StringIdentity idLinePermutationUBOName = stringIndexer().get("LinePermutationUBO");

void LineLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
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
    const auto getLinePatternPropsBuffer = [&]() {
        if (!linePatternPropertiesBuffer) {
            const LinePatternPropertiesUBO linePatternPropertiesUBO{
                /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
                /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
                /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
                /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
                /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
                0,
                0,
                0};
            linePatternPropertiesBuffer = context.createUniformBuffer(&linePatternPropertiesUBO,
                                                                      sizeof(linePatternPropertiesUBO));
        }
        return linePatternPropertiesBuffer;
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
    if (!expressionUniformBuffer) {
        const auto expressionUBO = buildExpressionUBO(zoom, parameters.frameCount);
        expressionUniformBuffer = context.createUniformBuffer(&expressionUBO, sizeof(expressionUBO));
    }
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        const auto shader = drawable.getShader();
        if (!drawable.getTileID() || !shader || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto& translation = evaluated.get<LineTranslate>();
        const auto anchor = evaluated.get<LineTranslateAnchor>();
        constexpr bool nearClipped = false;
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        auto& uniforms = drawable.mutableUniformBuffers();

        const auto matrix = getTileMatrix(tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits);

        const LineType type = static_cast<LineType>(drawable.getType());

        switch (type) {
            case LineType::Simple: {
                const LineUBO lineUBO{
                    /*matrix = */ util::cast<float>(matrix),
                    /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                    /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),
                    /*device_pixel_ratio = */ parameters.pixelRatio};
                uniforms.createOrUpdate(idLineUBOName, &lineUBO, context);

                // properties UBO
                uniforms.addOrReplace(idLinePropertiesUBOName, getLinePropsBuffer());

#if MLN_RENDER_BACKEND_METAL
                if (permutationUpdated) {
                    const LinePermutationUBO permutationUBO = {
                        /* .color = */ {/*.source=*/getAttributeSource<BuiltIn::LineShader>(2), /*.expression=*/{}},
                        /* .blur = */ {/*.source=*/getAttributeSource<BuiltIn::LineShader>(3), /*.expression=*/{}},
                        /* .opacity = */ {/*.source=*/getAttributeSource<BuiltIn::LineShader>(4), /*.expression=*/{}},
                        /* .gapwidth = */ {/*.source=*/getAttributeSource<BuiltIn::LineShader>(5), /*.expression=*/{}},
                        /* .offset = */ {/*.source=*/getAttributeSource<BuiltIn::LineShader>(6), /*.expression=*/{}},
                        /* .width = */ {/*.source=*/getAttributeSource<BuiltIn::LineShader>(7), /*.expression=*/{}},
                        /* .floorwidth = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .pattern_from = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .pattern_to = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .overdrawInspector = */ overdrawInspector,
                        /* .pad = */ 0,
                        0,
                        0,
                        0};

                    context.emplaceOrUpdateUniformBuffer(permutationUniformBuffer, &permutationUBO);
                    permutationUpdated = false;
                }
#endif
            } break;

            case LineType::Gradient: {
                const LineGradientUBO lineGradientUBO{
                    /*matrix = */ util::cast<float>(matrix),
                    /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                    /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),
                    /*device_pixel_ratio = */ parameters.pixelRatio};
                uniforms.createOrUpdate(idLineGradientUBOName, &lineGradientUBO, context);

                // properties UBO
                uniforms.addOrReplace(idLineGradientPropertiesUBOName, getLineGradientPropsBuffer());

#if MLN_RENDER_BACKEND_METAL
                if (permutationUpdated) {
                    const LinePermutationUBO permutationUBO = {
                        /* .color = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .blur = */
                        {/*.source=*/getAttributeSource<BuiltIn::LineGradientShader>(2), /*.expression=*/{}},
                        /* .opacity = */
                        {/*.source=*/getAttributeSource<BuiltIn::LineGradientShader>(3), /*.expression=*/{}},
                        /* .gapwidth = */
                        {/*.source=*/getAttributeSource<BuiltIn::LineGradientShader>(4), /*.expression=*/{}},
                        /* .offset = */
                        {/*.source=*/getAttributeSource<BuiltIn::LineGradientShader>(5), /*.expression=*/{}},
                        /* .width = */
                        {/*.source=*/getAttributeSource<BuiltIn::LineGradientShader>(6), /*.expression=*/{}},
                        /* .floorwidth = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .pattern_from = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .pattern_to = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .overdrawInspector = */ overdrawInspector,
                        /* .pad = */ 0,
                        0,
                        0,
                        0};

                    context.emplaceOrUpdateUniformBuffer(permutationUniformBuffer, &permutationUBO);
                    permutationUpdated = false;
                }
#endif
            } break;

            case LineType::Pattern: {
                Size textureSize{0, 0};
                if (const auto index = shader->getSamplerLocation(idTexImageName)) {
                    if (const auto& texture = drawable.getTexture(index.value())) {
                        textureSize = texture->getSize();
                    }
                }
                const LinePatternUBO linePatternUBO{
                    /*matrix =*/util::cast<float>(matrix),
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
                uniforms.createOrUpdate(idLinePatternUBOName, &linePatternUBO, context);

                // properties UBO
                uniforms.addOrReplace(idLinePatternPropertiesUBOName, getLinePatternPropsBuffer());

#if MLN_RENDER_BACKEND_METAL
                if (permutationUpdated) {
                    const LinePermutationUBO permutationUBO = {
                        /* .color = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .blur = */
                        {/*.source=*/getAttributeSource<BuiltIn::LinePatternShader>(2), /*.expression=*/{}},
                        /* .opacity = */
                        {/*.source=*/getAttributeSource<BuiltIn::LinePatternShader>(3), /*.expression=*/{}},
                        /* .gapwidth = */
                        {/*.source=*/getAttributeSource<BuiltIn::LinePatternShader>(4), /*.expression=*/{}},
                        /* .offset = */
                        {/*.source=*/getAttributeSource<BuiltIn::LinePatternShader>(5), /*.expression=*/{}},
                        /* .width = */
                        {/*.source=*/getAttributeSource<BuiltIn::LinePatternShader>(6), /*.expression=*/{}},
                        /* .floorwidth = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .pattern_from = */
                        {/*.source=*/getAttributeSource<BuiltIn::LinePatternShader>(7), /*.expression=*/{}},
                        /* .pattern_to = */
                        {/*.source=*/getAttributeSource<BuiltIn::LinePatternShader>(8), /*.expression=*/{}},
                        /* .overdrawInspector = */ overdrawInspector,
                        /* .pad = */ 0,
                        0,
                        0,
                        0};

                    context.emplaceOrUpdateUniformBuffer(permutationUniformBuffer, &permutationUBO);
                    permutationUpdated = false;
                }
#endif
            } break;

            case LineType::SDF: {
                if (const auto& data = drawable.getData()) {
                    const gfx::LineDrawableData& lineData = static_cast<const gfx::LineDrawableData&>(*data);
                    const auto& dashPatternTexture = parameters.lineAtlas.getDashPatternTexture(
                        evaluated.get<LineDasharray>().from,
                        evaluated.get<LineDasharray>().to,
                        lineData.linePatternCap);

                    // texture
                    if (const auto index = shader->getSamplerLocation(idTexImageName)) {
                        if (!drawable.getTexture(index.value())) {
                            const auto& texture = dashPatternTexture.getTexture();
                            drawable.setEnabled(!!texture);
                            if (texture) {
                                drawable.setTexture(texture, index.value());
                            }
                        }
                    }

                    const LinePatternPos& posA = dashPatternTexture.getFrom();
                    const LinePatternPos& posB = dashPatternTexture.getTo();
                    const float widthA = posA.width * crossfade.fromScale;
                    const float widthB = posB.width * crossfade.toScale;
                    const LineSDFUBO lineSDFUBO{
                        /* matrix = */ util::cast<float>(matrix),
                        {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                        {1.0f / tileID.pixelsToTileUnits(widthA, parameters.state.getIntegerZoom()),
                         -posA.height / 2.0f},
                        /* patternscale_b = */
                        {1.0f / tileID.pixelsToTileUnits(widthB, parameters.state.getIntegerZoom()),
                         -posB.height / 2.0f},
                        /* ratio = */ 1.0f /
                            tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom())),
                        /* device_pixel_ratio = */ parameters.pixelRatio,
                        /* tex_y_a = */ posA.y,
                        /* tex_y_b = */ posB.y,
                        /* sdfgamma = */ static_cast<float>(dashPatternTexture.getSize().width) /
                            (std::min(widthA, widthB) * 256.0f * parameters.pixelRatio) / 2.0f,
                        /* mix = */ crossfade.t};
                    uniforms.createOrUpdate(idLineSDFUBOName, &lineSDFUBO, context);

                    // properties UBO
                    uniforms.addOrReplace(idLineSDFPropertiesUBOName, getLineSDFPropsBuffer());
                }

#if MLN_RENDER_BACKEND_METAL
                if (permutationUpdated) {
                    const LinePermutationUBO permutationUBO = {
                        /* .color = */ {/*.source=*/getAttributeSource<BuiltIn::LineSDFShader>(2), /*.expression=*/{}},
                        /* .blur = */ {/*.source=*/getAttributeSource<BuiltIn::LineSDFShader>(3), /*.expression=*/{}},
                        /* .opacity = */
                        {/*.source=*/getAttributeSource<BuiltIn::LineSDFShader>(4), /*.expression=*/{}},
                        /* .gapwidth = */
                        {/*.source=*/getAttributeSource<BuiltIn::LineSDFShader>(5), /*.expression=*/{}},
                        /* .offset = */ {/*.source=*/getAttributeSource<BuiltIn::LineSDFShader>(6), /*.expression=*/{}},
                        /* .width = */ {/*.source=*/getAttributeSource<BuiltIn::LineSDFShader>(7), /*.expression=*/{}},
                        /* .floorwidth = */
                        {/*.source=*/getAttributeSource<BuiltIn::LineSDFShader>(8), /*.expression=*/{}},
                        /* .pattern_from = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .pattern_to = */ {/*.source=*/AttributeSource::Constant, /*.expression=*/{}},
                        /* .overdrawInspector = */ overdrawInspector,
                        /* .pad = */ 0,
                        0,
                        0,
                        0};

                    context.emplaceOrUpdateUniformBuffer(permutationUniformBuffer, &permutationUBO);
                    permutationUpdated = false;
                }
#endif
            } break;

            default: {
                using namespace std::string_literals;
                Log::Error(Event::General,
                           "LineLayerTweaker: unknown line type: "s + std::to_string(mbgl::underlying_type(type)));
            } break;
        }

#if MLN_RENDER_BACKEND_METAL
        uniforms.addOrReplace(idExpressionInputsUBOName, expressionUniformBuffer);
        uniforms.addOrReplace(idLinePermutationUBOName, permutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
    });
}

} // namespace mbgl
