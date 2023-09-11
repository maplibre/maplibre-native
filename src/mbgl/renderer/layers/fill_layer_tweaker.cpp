#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/std.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/fill.hpp>
#endif

namespace mbgl {

using namespace style;
using namespace shaders;

void FillLayerTweaker::execute(LayerGroupBase& layerGroup,
                               const RenderTree& renderTree,
                               const PaintParameters& parameters) {
    auto& context = parameters.context;
    const auto& props = static_cast<const FillLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;
    const auto& crossfade = props.crossfade;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

#if MLN_RENDER_BACKEND_METAL
    const auto source = [this](const std::string_view& attrName) {
        return hasPropertyAsUniform(attrName) ? AttributeSource::Constant : AttributeSource::PerVertex;
    };
#endif

    bool fillUniformBufferUpdated = false;
    bool fillOutlineUniformBufferUpdated = false;
    bool fillPatternUniformBufferUpdated = false;
    bool fillOutlinePatternUniformBufferUpdated = false;

    auto UpdateFillUniformBuffers = [&]() {
        if (fillUniformBufferUpdated) return;

#if MLN_RENDER_BACKEND_METAL
        using ShaderClass = ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>;
        if (propertiesChanged || !fillPermutationUniformBuffer) {
            const FillPermutationUBO permutationUBO = {
                /* .color = */ {/*.source=*/source(ShaderClass::attributes[1].name), /*.expression=*/{}},
                /* .opacity = */ {/*.source=*/source(ShaderClass::attributes[2].name), /*.expression=*/{}},
                /* .overdrawInspector = */ overdrawInspector,
                0,
                0,
                0,
                0,
                0,
                0,
            };

            if (fillPermutationUniformBuffer) {
                fillPermutationUniformBuffer->update(&permutationUBO, sizeof(permutationUBO));
            } else {
                fillPermutationUniformBuffer = context.createUniformBuffer(&permutationUBO, sizeof(permutationUBO));
            }
            fillUniformBufferUpdated = true;
        }
#endif

        if (!fillPropsUniformBuffer) {
            const FillEvaluatedPropsUBO paramsUBO = {
                /* .color = */ evaluated.get<FillColor>().constantOr(FillColor::defaultValue()),
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                0,
                0,
                0,
            };
            fillPropsUniformBuffer = context.createUniformBuffer(&paramsUBO, sizeof(paramsUBO));
        }
    };

    auto UpdateFillOutlineUniformBuffers = [&]() {
        if (fillOutlineUniformBufferUpdated) return;

#if MLN_RENDER_BACKEND_METAL
        using ShaderClass = ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>;
        if (propertiesChanged || !fillOutlinePermutationUniformBuffer) {
            const FillOutlinePermutationUBO permutationUBO = {
                /* .outline_color = */ {/*.source=*/source(ShaderClass::attributes[1].name), /*.expression=*/{}},
                /* .opacity = */ {/*.source=*/source(ShaderClass::attributes[2].name), /*.expression=*/{}},
                /* .overdrawInspector = */ overdrawInspector,
                0,
                0,
                0,
                0,
                0,
                0,
            };

            if (fillOutlinePermutationUniformBuffer) {
                fillOutlinePermutationUniformBuffer->update(&permutationUBO, sizeof(permutationUBO));
            } else {
                fillOutlinePermutationUniformBuffer = context.createUniformBuffer(&permutationUBO,
                                                                                  sizeof(permutationUBO));
            }
        }
#endif

        if (!fillOutlinePropsUniformBuffer) {
            const FillOutlineEvaluatedPropsUBO paramsUBO = {
                /* .outline_color = */ evaluated.get<FillOutlineColor>().constantOr(FillOutlineColor::defaultValue()),
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                0,
                0,
                0,
            };
            fillOutlinePropsUniformBuffer = context.createUniformBuffer(&paramsUBO, sizeof(paramsUBO));
        }

        fillOutlineUniformBufferUpdated = true;
    };

    auto UpdateFillPatternUniformBuffers = [&]() {
        if (fillPatternUniformBufferUpdated) return;

#if MLN_RENDER_BACKEND_METAL
        using ShaderClass = ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>;
        if (propertiesChanged || !fillPatternPermutationUniformBuffer) {
            const FillPatternPermutationUBO permutationUBO = {
                /* .pattern_from = */ {/*.source=*/source(ShaderClass::attributes[1].name), /*.expression=*/{}},
                /* .pattern_to = */ {/*.source=*/source(ShaderClass::attributes[2].name), /*.expression=*/{}},
                /* .opacity = */ {/*.source=*/source(ShaderClass::attributes[3].name), /*.expression=*/{}},
                /* .overdrawInspector = */ overdrawInspector,
                0,
                0,
                0,
                0,
            };

            if (fillPatternPermutationUniformBuffer) {
                fillPatternPermutationUniformBuffer->update(&permutationUBO, sizeof(permutationUBO));
            } else {
                fillPatternPermutationUniformBuffer = context.createUniformBuffer(&permutationUBO,
                                                                                  sizeof(permutationUBO));
            }
        }
#endif

        if (!fillPatternPropsUniformBuffer) {
            const FillPatternEvaluatedPropsUBO paramsUBO = {
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                /* .fade = */ crossfade.t,
                0,
                0,
            };
            fillPatternPropsUniformBuffer = context.createUniformBuffer(&paramsUBO, sizeof(paramsUBO));
        }

        fillPatternUniformBufferUpdated = true;
    };

    auto UpdateFillOutlinePatternUniformBuffers = [&]() {
        if (fillOutlinePatternUniformBufferUpdated) return;

#if MLN_RENDER_BACKEND_METAL
        using ShaderClass = ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>;
        if (propertiesChanged || !fillOutlinePermutationUniformBuffer) {
            const FillOutlinePatternPermutationUBO permutationUBO = {
                /* .pattern_from = */ {/*.source=*/source(ShaderClass::attributes[1].name), /*.expression=*/{}},
                /* .pattern_to = */ {/*.source=*/source(ShaderClass::attributes[2].name), /*.expression=*/{}},
                /* .opacity = */ {/*.source=*/source(ShaderClass::attributes[3].name), /*.expression=*/{}},
                /* .overdrawInspector = */ overdrawInspector,
                0,
                0,
                0,
                0,
            };

            if (fillOutlinePatternPermutationUniformBuffer) {
                fillOutlinePatternPermutationUniformBuffer->update(&permutationUBO, sizeof(permutationUBO));
            } else {
                fillOutlinePatternPermutationUniformBuffer = context.createUniformBuffer(&permutationUBO,
                                                                                         sizeof(permutationUBO));
            }
        }
#endif

        if (!fillOutlinePatternPropsUniformBuffer) {
            const FillOutlinePatternEvaluatedPropsUBO paramsUBO = {
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                /* .fade = */ crossfade.t,
                0,
                0,
            };
            fillOutlinePatternPropsUniformBuffer = context.createUniformBuffer(&paramsUBO, sizeof(paramsUBO));
        }

        fillOutlinePatternUniformBufferUpdated = true;
    };

#if MLN_RENDER_BACKEND_METAL
    if (!expressionUniformBuffer) {
        const auto zoom = parameters.state.getZoom();
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
        auto& uniforms = drawable.mutableUniformBuffers();
        // auto& drawableType = drawable.getShader()->typeName();

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto& translation = evaluated.get<FillTranslate>();
        const auto anchor = evaluated.get<FillTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = false;
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, nearClipped, inViewportPixelUnits);

        // from FillPatternProgram::layoutUniformValues
        const auto renderableSize = parameters.backend.getDefaultRenderable().getSize();
        const auto intZoom = parameters.state.getIntegerZoom();
        const auto tileRatio = 1.0f / tileID.pixelsToTileUnits(1.0f, intZoom);
        const int32_t tileSizeAtNearestZoom = static_cast<int32_t>(
            util::tileSize_D * parameters.state.zoomScale(intZoom - tileID.canonical.z));
        const int32_t pixelX = static_cast<int32_t>(
            tileSizeAtNearestZoom *
            (tileID.canonical.x + tileID.wrap * parameters.state.zoomScale(tileID.canonical.z)));
        const int32_t pixelY = tileSizeAtNearestZoom * tileID.canonical.y;
        const auto pixelRatio = parameters.pixelRatio;

        Size textureSize = {0, 0};
        if (const auto shader = drawable.getShader()) {
            if (const auto index = shader->getSamplerLocation("u_image")) {
                if (const auto& tex = drawable.getTexture(*index)) {
                    textureSize = tex->getSize();
                }
            }
        }
        if (!drawable.getTileID()) {
            return;
        }

        if (uniforms.get("FillInterpolateUBO")) {
            UpdateFillUniformBuffers();

            uniforms.addOrReplace("FillEvaluatedPropsUBO", fillPropsUniformBuffer);
            const FillDrawableUBO drawableUBO = {/*.matrix=*/util::cast<float>(matrix)};

            uniforms.createOrUpdate("FillDrawableUBO", &drawableUBO, context);

#if MLN_RENDER_BACKEND_METAL
            uniforms.addOrReplace("FillPermutationUBO", fillPermutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
        } else if (uniforms.get("FillOutlineInterpolateUBO")) {
            UpdateFillOutlineUniformBuffers();

            uniforms.addOrReplace("FillOutlineEvaluatedPropsUBO", fillOutlinePropsUniformBuffer);
            const FillOutlineDrawableUBO drawableUBO = {
                /*.matrix=*/util::cast<float>(matrix),
                /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
            };

            uniforms.createOrUpdate("FillOutlineDrawableUBO", &drawableUBO, context);

#if MLN_RENDER_BACKEND_METAL
            uniforms.addOrReplace("FillOutlinePermutationUBO", fillOutlinePermutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
        } else if (uniforms.get("FillPatternInterpolateUBO")) {
            UpdateFillPatternUniformBuffers();

            uniforms.addOrReplace("FillPatternEvaluatedPropsUBO", fillPatternPropsUniformBuffer);
            const FillPatternDrawableUBO drawableUBO = {
                /*.matrix=*/util::cast<float>(matrix),
                /*.scale=*/{pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
                /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                0,
                0,
            };

            uniforms.createOrUpdate("FillPatternDrawableUBO", &drawableUBO, context);

#if MLN_RENDER_BACKEND_METAL
            uniforms.addOrReplace("FillPatternPermutationUBO", fillPatternPermutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
        } else if (uniforms.get("FillOutlinePatternInterpolateUBO")) {
            UpdateFillOutlinePatternUniformBuffers();

            uniforms.addOrReplace("FillOutlinePatternEvaluatedPropsUBO", fillOutlinePatternPropsUniformBuffer);
            const FillOutlinePatternDrawableUBO drawableUBO = {
                /*.matrix=*/util::cast<float>(matrix),
                /*.scale=*/{pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
                /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
                /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
            };

            uniforms.createOrUpdate("FillOutlinePatternDrawableUBO", &drawableUBO, context);

#if MLN_RENDER_BACKEND_METAL
            uniforms.addOrReplace("FillOutlinePatternPermutationUBO", fillOutlinePatternPermutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
        }

#if MLN_RENDER_BACKEND_METAL
        uniforms.addOrReplace("ExpressionInputsUBO", expressionUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
    });

    propertiesChanged = false;
}

} // namespace mbgl
