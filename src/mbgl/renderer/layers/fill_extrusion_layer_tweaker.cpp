#include <mbgl/renderer/layers/fill_extrusion_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/programs/fill_extrusion_program.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/string_indexer.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/fill_extrusion.hpp>
#endif

namespace mbgl {

using namespace shaders;
using namespace style;

namespace {
const StringIdentity idFillExtrusionDrawableUBOName = stringIndexer().get("FillExtrusionDrawableUBO");
const StringIdentity idFillExtrusionDrawablePropsUBOName = stringIndexer().get("FillExtrusionDrawablePropsUBO");
const StringIdentity idExpressionInputsUBOName = stringIndexer().get("ExpressionInputsUBO");
const StringIdentity idFillExtrusionPermutationUBOName = stringIndexer().get("FillExtrusionPermutationUBO");
const StringIdentity idTexImageName = stringIndexer().get("u_image");

} // namespace

const StringIdentity FillExtrusionLayerTweaker::idFillExtrusionTilePropsUBOName = stringIndexer().get(
    "FillExtrusionDrawableTilePropsUBO");
const StringIdentity FillExtrusionLayerTweaker::idFillExtrusionInterpolateUBOName = stringIndexer().get(
    "FillExtrusionInterpolateUBO");

void FillExtrusionLayerTweaker::execute(LayerGroupBase& layerGroup,
                                        const RenderTree& renderTree,
                                        const PaintParameters& parameters) {
    auto& context = parameters.context;
    const auto& props = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;
    const auto& crossfade = props.crossfade;
    const auto& state = parameters.state;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    // UBO depends on more than just evaluated properties, so we need to update every time,
    // but the resulting buffer can be shared across all the drawables from the layer.
    const FillExtrusionDrawablePropsUBO paramsUBO = {
        /* .color = */ constOrDefault<FillExtrusionColor>(evaluated),
        /* .light_color = */ FillExtrusionProgram::lightColor(parameters.evaluatedLight),
        /* .pad = */ 0,
        /* .light_position = */ FillExtrusionProgram::lightPosition(parameters.evaluatedLight, state),
        /* .base = */ constOrDefault<FillExtrusionBase>(evaluated),
        /* .height = */ constOrDefault<FillExtrusionHeight>(evaluated),
        /* .light_intensity = */ FillExtrusionProgram::lightIntensity(parameters.evaluatedLight),
        /* .vertical_gradient = */ evaluated.get<FillExtrusionVerticalGradient>() ? 1.0f : 0.0f,
        /* .opacity = */ evaluated.get<FillExtrusionOpacity>(),
        /* .fade = */ crossfade.t,
        /* .pad = */ 0,
        0,
        0};
    if (!propsBuffer) {
        propsBuffer = context.createUniformBuffer(&paramsUBO, sizeof(paramsUBO));
    } else {
        propsBuffer->update(&paramsUBO, sizeof(paramsUBO));
    }

#if MLN_RENDER_BACKEND_METAL
    const auto zoom = parameters.state.getZoom();
    if (permutationUpdated) {
        const FillExtrusionPermutationUBO permutationUBO = {
            /* .color = */ {/*.source=*/getAttributeSource<BuiltIn::FillExtrusionShader>(2), /*.expression=*/{}},
            /* .base = */ {/*.source=*/getAttributeSource<BuiltIn::FillExtrusionShader>(3), /*.expression=*/{}},
            /* .height = */ {/*.source=*/getAttributeSource<BuiltIn::FillExtrusionShader>(4), /*.expression=*/{}},
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
        permutationUpdated = false;
    }
    if (!expressionUniformBuffer) {
        const auto expressionUBO = buildExpressionUBO(zoom, parameters.frameCount);
        expressionUniformBuffer = context.createUniformBuffer(&expressionUBO, sizeof(expressionUBO));
    }
#endif

    layerGroup.visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.addOrReplace(idFillExtrusionDrawablePropsUBOName, propsBuffer);

        const auto& translation = evaluated.get<FillExtrusionTranslate>();
        const auto anchor = evaluated.get<FillExtrusionTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = true;
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, nearClipped, inViewportPixelUnits);

        const auto tileRatio = 1 / tileID.pixelsToTileUnits(1, state.getIntegerZoom());
        const auto zoomScale = state.zoomScale(tileID.canonical.z);
        const auto nearestZoomScale = state.zoomScale(state.getIntegerZoom() - tileID.canonical.z);
        const auto tileSizeAtNearestZoom = std::floor(util::tileSize_D * nearestZoomScale);
        const auto pixelX = static_cast<int32_t>(tileSizeAtNearestZoom *
                                                 (tileID.canonical.x + tileID.wrap * zoomScale));
        const auto pixelY = static_cast<int32_t>(tileSizeAtNearestZoom * tileID.canonical.y);
        const auto pixelRatio = parameters.pixelRatio;
        const auto numTiles = std::pow(2, tileID.canonical.z);
        const auto heightFactor = static_cast<float>(-numTiles / util::tileSize_D / 8.0);

        Size textureSize = {0, 0};
        if (const auto shader = drawable.getShader()) {
            if (const auto index = shader->getSamplerLocation(idTexImageName)) {
                if (const auto& tex = drawable.getTexture(*index)) {
                    textureSize = tex->getSize();
                }
            }
        }

        const FillExtrusionDrawableUBO drawableUBO = {
            /* .matrix = */ util::cast<float>(matrix),
            /* .scale = */ {pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
            /* .texsize = */ {static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
            /* .pixel_coord_upper = */ {static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
            /* .pixel_coord_lower = */ {static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
            /* .height_factor = */ heightFactor,
            /* .pad = */ 0};

        uniforms.createOrUpdate(idFillExtrusionDrawableUBOName, &drawableUBO, context);

#if MLN_RENDER_BACKEND_METAL
        uniforms.addOrReplace(idExpressionInputsUBOName, expressionUniformBuffer);
        uniforms.addOrReplace(idFillExtrusionPermutationUBOName, permutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
    });
}

} // namespace mbgl
