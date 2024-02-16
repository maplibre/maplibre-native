#include <mbgl/renderer/layers/line_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/line_drawable_data.hpp>
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

void LineLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    auto& context = parameters.context;
    const auto zoom = parameters.state.getZoom();
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;
    const auto& crossfade = static_cast<const LineLayerProperties&>(*evaluatedProperties).crossfade;

    // Each property UBO is updated at most once if new evaluated properties were set
    if (propertiesUpdated) {
        simplePropertiesUpdated = true;
        gradientPropertiesUpdated = true;
        patternPropertiesUpdated = true;
        sdfPropertiesUpdated = true;
        propertiesUpdated = false;
    }

    const auto getLinePropsBuffer = [&]() {
        if (!linePropertiesBuffer || simplePropertiesUpdated) {
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
            context.emplaceOrUpdateUniformBuffer(linePropertiesBuffer, &linePropertiesUBO);
            simplePropertiesUpdated = false;
        }
        return linePropertiesBuffer;
    };
    const auto getLineGradientPropsBuffer = [&]() {
        if (!lineGradientPropertiesBuffer || gradientPropertiesUpdated) {
            const LineGradientPropertiesUBO lineGradientPropertiesUBO{
                /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
                /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
                /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
                /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
                /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
                0,
                0,
                0};
            context.emplaceOrUpdateUniformBuffer(lineGradientPropertiesBuffer, &lineGradientPropertiesUBO);
            gradientPropertiesUpdated = false;
        }
        return lineGradientPropertiesBuffer;
    };
    const auto getLinePatternPropsBuffer = [&]() {
        if (!linePatternPropertiesBuffer || patternPropertiesUpdated) {
            const LinePatternPropertiesUBO linePatternPropertiesUBO{
                /*blur =*/evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
                /*opacity =*/evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
                /*offset =*/evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
                /*gapwidth =*/evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
                /*width =*/evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
                0,
                0,
                0};
            context.emplaceOrUpdateUniformBuffer(linePatternPropertiesBuffer, &linePatternPropertiesUBO);
            patternPropertiesUpdated = false;
        }
        return linePatternPropertiesBuffer;
    };
    const auto getLineSDFPropsBuffer = [&]() {
        if (!lineSDFPropertiesBuffer || sdfPropertiesUpdated) {
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
            context.emplaceOrUpdateUniformBuffer(lineSDFPropertiesBuffer, &lineSDFPropertiesUBO);
            sdfPropertiesUpdated = false;
        }
        return lineSDFPropertiesBuffer;
    };

    const LineDynamicUBO dynamicUBO = {
        /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]}, 0, 0};
    context.emplaceOrUpdateUniformBuffer(dynamicBuffer, &dynamicUBO);

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

        const auto matrix = getTileMatrix(
            tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits, drawable);

        const LineType type = static_cast<LineType>(drawable.getType());
        switch (type) {
            case LineType::Simple: {
                const LineUBO lineUBO{/*matrix = */ util::cast<float>(matrix),
                                      /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),
                                      0,
                                      0,
                                      0};
                uniforms.createOrUpdate(idLineUBO, &lineUBO, context);

                // properties UBO
                uniforms.set(idLinePropertiesUBO, getLinePropsBuffer());

                // dynamic UBO
                uniforms.set(idLineDynamicUBO, dynamicBuffer);
            } break;

            case LineType::Gradient: {
                const LineGradientUBO lineGradientUBO{
                    /*matrix = */ util::cast<float>(matrix),
                    /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),
                    0,
                    0,
                    0};
                uniforms.createOrUpdate(idLineGradientUBO, &lineGradientUBO, context);

                // properties UBO
                uniforms.set(idLineGradientPropertiesUBO, getLineGradientPropsBuffer());

                // dynamic UBO
                uniforms.set(idLineGradientDynamicUBO, dynamicBuffer);
            } break;

            case LineType::Pattern: {
                Size textureSize{0, 0};
                if (const auto& texture = drawable.getTexture(idLineImageTexture)) {
                    textureSize = texture->getSize();
                }
                const LinePatternUBO linePatternUBO{
                    /*matrix =*/util::cast<float>(matrix),
                    /*scale =*/
                    {parameters.pixelRatio,
                     1 / tileID.pixelsToTileUnits(1, parameters.state.getIntegerZoom()),
                     crossfade.fromScale,
                     crossfade.toScale},
                    /*texsize =*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                    /*ratio =*/1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),
                    /*fade =*/crossfade.t};
                uniforms.createOrUpdate(idLinePatternUBO, &linePatternUBO, context);

                // properties UBO
                uniforms.set(idLinePatternPropertiesUBO, getLinePatternPropsBuffer());

                // dynamic UBO
                uniforms.set(idLinePatternDynamicUBO, dynamicBuffer);

            } break;

            case LineType::SDF: {
                if (const auto& data = drawable.getData()) {
                    const gfx::LineDrawableData& lineData = static_cast<const gfx::LineDrawableData&>(*data);
                    const auto& dashPatternTexture = parameters.lineAtlas.getDashPatternTexture(
                        evaluated.get<LineDasharray>().from,
                        evaluated.get<LineDasharray>().to,
                        lineData.linePatternCap);

                    // texture
                    if (!drawable.getTexture(idLineImageTexture)) {
                        const auto& texture = dashPatternTexture.getTexture();
                        drawable.setEnabled(!!texture);
                        if (texture) {
                            drawable.setTexture(texture, idLineImageTexture);
                        }
                    }

                    const LinePatternPos& posA = dashPatternTexture.getFrom();
                    const LinePatternPos& posB = dashPatternTexture.getTo();
                    const float widthA = posA.width * crossfade.fromScale;
                    const float widthB = posB.width * crossfade.toScale;
                    const LineSDFUBO lineSDFUBO{
                        /* matrix = */ util::cast<float>(matrix),
                        /* patternscale_a = */
                        {1.0f / tileID.pixelsToTileUnits(widthA, parameters.state.getIntegerZoom()),
                         -posA.height / 2.0f},
                        /* patternscale_b = */
                        {1.0f / tileID.pixelsToTileUnits(widthB, parameters.state.getIntegerZoom()),
                         -posB.height / 2.0f},
                        /* ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),
                        /* tex_y_a = */ posA.y,
                        /* tex_y_b = */ posB.y,
                        /* sdfgamma = */ static_cast<float>(dashPatternTexture.getSize().width) /
                            (std::min(widthA, widthB) * 256.0f * parameters.pixelRatio) / 2.0f,
                        /* mix = */ crossfade.t,
                        0,
                        0,
                        0};
                    uniforms.createOrUpdate(idLineSDFUBO, &lineSDFUBO, context);

                    // properties UBO
                    uniforms.set(idLineSDFPropertiesUBO, getLineSDFPropsBuffer());

                    // dynamic UBO
                    uniforms.set(idLineSDFDynamicUBO, dynamicBuffer);
                }
            } break;

            default: {
                using namespace std::string_literals;
                Log::Error(Event::General,
                           "LineLayerTweaker: unknown line type: "s + std::to_string(mbgl::underlying_type(type)));
            } break;
        }
    });
}

} // namespace mbgl
