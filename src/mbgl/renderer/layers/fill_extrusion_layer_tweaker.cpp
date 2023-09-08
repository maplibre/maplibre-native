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
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/string_indexer.hpp>

namespace mbgl {

using namespace style;

namespace {

struct alignas(16) FillExtrusionDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 4> scale;
    /*  80 */ std::array<float, 2> texsize;
    /*  88 */ std::array<float, 2> pixel_coord_upper;
    /*  96 */ std::array<float, 2> pixel_coord_lower;
    /* 104 */ float height_factor;
    /* 108 */ float pad;
    /* 112 */
};
static_assert(sizeof(FillExtrusionDrawableUBO) == 7 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) FillExtrusionDrawablePropsUBO {
    /*  0 */ Color color;
    /* 16 */ std::array<float, 3> light_color;
    /* 28 */ float pad1;
    /* 32 */ std::array<float, 3> light_position;
    /* 44 */ float base;
    /* 48 */ float height;
    /* 52 */ float light_intensity;
    /* 56 */ float vertical_gradient;
    /* 60 */ float opacity;
    /* 64 */ float fade;
    /* 68 */ float pad2, pad3, pad4;
    /* 80 */
};
static_assert(sizeof(FillExtrusionDrawablePropsUBO) == 5 * 16);

static const StringIdentity idFillExtrusionDrawableUBOName = StringIndexer::get("FillExtrusionDrawableUBO");
static const StringIdentity idFillExtrusionDrawablePropsUBOName = StringIndexer::get("FillExtrusionDrawablePropsUBO");
static const StringIdentity idTexImageName = StringIndexer::get("u_image");

template <typename T, class... Is, class... Ts>
auto constOrDefault(const IndexedTuple<TypeList<Is...>, TypeList<Ts...>>& evaluated) {
    return evaluated.template get<T>().constantOr(T::defaultValue());
}

} // namespace

const StringIdentity FillExtrusionLayerTweaker::idFillExtrusionTilePropsUBOName = StringIndexer::get(
    "FillExtrusionDrawableTilePropsUBO");
const StringIdentity FillExtrusionLayerTweaker::idFillExtrusionInterpolateUBOName = StringIndexer::get(
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

    layerGroup.visitDrawables([&](gfx::Drawable& drawable) {
        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.addOrReplace(idFillExtrusionDrawablePropsUBOName, propsBuffer);

        if (!drawable.getTileID()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

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
    });
}

} // namespace mbgl
