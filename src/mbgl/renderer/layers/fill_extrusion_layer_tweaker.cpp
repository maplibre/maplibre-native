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
#include <mbgl/shaders/mtl/fill_extrusion_pattern.hpp>
#endif

namespace mbgl {

using namespace shaders;
using namespace style;

namespace {
const StringIdentity idTexImageName = stringIndexer().get("u_image");

} // namespace

void FillExtrusionLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
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
    context.emplaceOrUpdateUniformBuffer(propsBuffer, &paramsUBO);
    propertiesUpdated = false;

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.addOrReplace(idFillExtrusionDrawablePropsUBO, propsBuffer);

        const auto& translation = evaluated.get<FillExtrusionTranslate>();
        const auto anchor = evaluated.get<FillExtrusionTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = true;
        const auto matrix = getTileMatrix(
            tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits, drawable);

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

        uniforms.createOrUpdate(idFillExtrusionDrawableUBO, &drawableUBO, context);
    });
}

} // namespace mbgl
